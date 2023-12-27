// USAR DOIS WORKERS E UMA VARIAVEL DE CONTAGEM DE UTILIZADORES
#include "bathrooms.h"
#include "../common/common.h"
#include "slist.h"
#include "user.h"
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>

extern bool parque_aberto;

bool enter_bathrooms(user_info *info) {

  // Determina se houve um acidente
  bool accident = false;

  if (info->deficient) {
    accident = disabled_wc(info);
  } else if (info->is_man) {
    // TODO Casas de banho duplas de homens
    // Tentar usar ao máximo o que já foi feito
    accident = men_wc(info);
  } else if (!info->is_man) {
    // Casas de banho duplas de mulheres
    accident = women_wc(info);
  }

  return accident;
}

//-----------------------------------------------------------------
//                 CASA DE BANHO DE DEFICIENTES
//-----------------------------------------------------------------

// Lista de espera para a casa de banho dos deficientes
struct queue_head deficient_restroom_queue =
    SLIST_HEAD_INITIALIZER(deficient_restroom_queue);

// Controla que só um user pode estar a entrar ou sair da casa de banho
pthread_mutex_t deficient_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

// Age como um trinco duplo, para que um utilizador possa
// sair da casa de banho sem ser interrompido
sem_t user_done_def_sem, worker_done_def_sem;

void disabled_bathroom_worker_entry_point() {
  sem_init(&user_done_def_sem, 0, 0);
  sem_init(&worker_done_def_sem, 0, 0);

  SLIST_INIT(&deficient_restroom_queue);

  // TODO remove busy waiting
  while (parque_aberto) {
    pthread_mutex_lock(&deficient_queue_mutex);

    // Tentar libertar a head da lista de utilizadores há espera
    struct queue_item *head = SLIST_FIRST(&deficient_restroom_queue);

    if (head != NULL) {

      // Fazer com que o user utilize a atração
      SLIST_REMOVE_HEAD(&deficient_restroom_queue, entries);

      // Chance de 1 em 1000 de haver um acidente
      if (rand() % 1000 == 0) {
        head->left_state = ACCIDENT;
      }
      sem_post(&head->semaphore);

      sem_wait(&user_done_def_sem);
      sem_post(&worker_done_def_sem);

      // Sempre que um utilizador sai na casa de banho, rolar uma chance de os
      // outros desistirem

      struct queue_item *user = NULL;
      SLIST_FOREACH(user, &deficient_restroom_queue, entries) {
        if (user) {
          // TODO Adicionar chance de desistência a um parametro no ficheiro de
          // configuração
          // TODO Mudar a heuristica de desistência
          if (rand() % 20 == 0 && slist_length(&deficient_restroom_queue) > 0) {
            SLIST_REMOVE(&deficient_restroom_queue, user, queue_item, entries);
            user->left_state = QUIT;
            sem_post(&user->semaphore);

            sem_wait(&user_done_def_sem);
            sem_post(&worker_done_def_sem);
          }
        }
      }
    }
    pthread_mutex_unlock(&deficient_queue_mutex);
  };

  struct queue_item *user = NULL;

  // Libertar todos os utilizadores na fila de espera quando o parque fecha
  while (user) {
    user->left_state = QUIT;
    sem_post(&user->semaphore);

    sem_wait(&user_done_def_sem);
    sem_post(&worker_done_def_sem);
  };
}

bool disabled_wc(user_info *info) {

  // Criar entrada deste user na lista de espera da casa de banho
  struct queue_item *entry = malloc(sizeof(struct queue_item));
  sem_init(&entry->semaphore, 0, 0);
  entry->entries.sle_next = NULL;
  entry->left_state = RUNNING;

  pthread_mutex_lock(&deficient_queue_mutex);

  insert_at_end_of_slist(&deficient_restroom_queue, entry);

  pthread_mutex_unlock(&deficient_queue_mutex);


  // Mensagem de entrada de user na casa de banho dos deficientes
  char buffer[MAX_MESSAGE_BUFFER_SIZE];
  snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE - 1, "%d", info->i);
  thread_send_message_to_socket(info->socket_monitor, ENWCD, buffer);

  // Esperar pela vez do utilizador para entrar na atração
  sem_wait(&entry->semaphore);

  // Lock duplo para que só um utilizador esteja a sair da casa de banho num
  // determinado instante
  sem_post(&user_done_def_sem);

  // Todos os 3 tipos de mensagens que podem ser enviadas, vão enviar o id
  snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE - 1, "%d", info->i);

  MessageType type;
  bool accident = entry->left_state == ACCIDENT;

  // Determinar o que fazer assim que o utilizador saia da atração
  if (entry->left_state == RUNNING)
    type = EXWCD; // User usou e saiu da casa de banho
  else if (entry->left_state == QUIT)
    type = DESIS; // User desistiu da fila de espera
  else if (entry->left_state == ACCIDENT)
    type = ACCID; // User teve um acidente e tem que sair do parque

  // Enviar mensagem para o monitor
  thread_send_message_to_socket(info->socket_monitor, type, buffer);

  sem_destroy(&entry->semaphore);
  free(entry);

  sem_wait(&worker_done_def_sem);

  return accident;
}

//-----------------------------------------------------------------
//               FIM CASA DE BANHO DE DEFICIENTES
//-----------------------------------------------------------------

//-----------------------------------------------------------------
//                 CASA DE BANHO DE HOMENS
//-----------------------------------------------------------------

// Lista de espera para a casa de banho dos deficientes
struct queue_head men_restroom_queue =
    SLIST_HEAD_INITIALIZER(men_restroom_queue);
struct queue_head men_restroom_vip_queue =
    SLIST_HEAD_INITIALIZER(men_restroom_vip_queue);

// Controla que só um user pode estar a entrar ou sair da casa de banho
pthread_mutex_t men_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

// Exclusão mútua, variável number_of_people_in_men_bathrooms
pthread_mutex_t number_people_in_men_bathroom_mutex = PTHREAD_MUTEX_INITIALIZER;

// Controla quantas pessoas podem estar dentro das casas de banho dos homens
int number_of_people_in_men_bahrooms = 0;

// Neste caso, duas podem estar, pois há duas casas de banho
#define LIMIT_PEOPLE_IN_MEN_BATHROOMS 2

// Age como um trinco duplo, para que um utilizador possa
// sair da casa de banho sem ser interrompido
sem_t user_done_men_sem, worker_done_men_sem;

void men_bathroom_worker_entry_point() {
  sem_init(&user_done_men_sem, 0, 0);
  sem_init(&worker_done_men_sem, 0, 0);

  SLIST_INIT(&men_restroom_queue);
  SLIST_INIT(&men_restroom_vip_queue);

  // TODO remove busy waiting
  while (parque_aberto) {

    pthread_mutex_lock(&men_queue_mutex);
    // Não libertar pessoas da fila se existem mais que duas pessoas nas casas
    // de banho
    if (number_of_people_in_men_bahrooms >= LIMIT_PEOPLE_IN_MEN_BATHROOMS){
      continue;
    }

    // Tentar libertar primeiro a lista dos vips antes de deixar os outros
    // passarem
    struct queue_item *head = NULL;

    if (!SLIST_EMPTY(&men_restroom_vip_queue)) {
      head = SLIST_FIRST(&men_restroom_vip_queue);
      SLIST_REMOVE_HEAD(&men_restroom_vip_queue, entries);
    } else if (!SLIST_EMPTY(&men_restroom_queue)) {
      head = SLIST_FIRST(&men_restroom_queue);
      SLIST_REMOVE_HEAD(&men_restroom_queue, entries);
    }
    pthread_mutex_unlock(&men_queue_mutex);

    if (head != NULL) {

      // Chance de 1 em 1000 de haver um acidente
      if (rand() % 1000 == 0) {
        head->left_state = ACCIDENT;
      }

      // Fazer um user entrar na casa de banho
      number_of_people_in_men_bahrooms++;

      sem_post(&head->semaphore);

      sem_wait(&user_done_men_sem);
      // User a sair da casa de banho

      number_of_people_in_men_bahrooms--;
      // Sempre que um utilizador sai na casa de banho, rolar uma chance de os
      // outros desistirem
      // TODO Desistências
      struct queue_item *user = NULL;

      sem_post(&worker_done_men_sem);

      // Rola uma chance para que, individualmente, todos os utilizadores
      // desistam das filas de espera
      pthread_mutex_lock(&men_queue_mutex);
      if (!(SLIST_EMPTY(&men_restroom_queue))){
        SLIST_FOREACH(user, &men_restroom_queue, entries) {
          if (!user)
            continue;

          if (rand() % 20 == 0 && user != NULL) {
            SLIST_REMOVE(&men_restroom_queue, user, queue_item, entries);
            user->left_state = QUIT;
            sem_post(&user->semaphore);

            sem_wait(&user_done_men_sem);
            sem_post(&worker_done_men_sem);
          }
        }
      }

      user = NULL;
      if (!(SLIST_EMPTY(&men_restroom_vip_queue))){
	SLIST_FOREACH(user, &men_restroom_vip_queue, entries) {
	  if (!user)
	    continue;

          if (rand() % 20 == 0 && user != NULL) {
            SLIST_REMOVE(&men_restroom_vip_queue, user, queue_item, entries);
            user->left_state = QUIT;
            sem_post(&user->semaphore);

            sem_wait(&user_done_men_sem);
            sem_post(&worker_done_men_sem);
          }
        }
      }
      pthread_mutex_unlock(&men_queue_mutex);
    }
  };

  struct queue_item *user = NULL;

  // Libertar todos os utilizadores na fila de espera quando o parque fecha
  while (user) {
    user->left_state = QUIT;
    sem_post(&user->semaphore);

    sem_wait(&user_done_men_sem);
    sem_post(&worker_done_men_sem);

    user = user->entries.sle_next;
  };

  user = SLIST_FIRST(&men_restroom_vip_queue);
  while (user) {
    user->left_state = QUIT;
    sem_post(&user->semaphore);

    sem_wait(&user_done_men_sem);
    sem_post(&worker_done_men_sem);

    user = user->entries.sle_next;
  };
}

bool men_wc(user_info *info) {

  // Criar entrada deste user na lista de espera da casa de banho
  struct queue_item *entry = malloc(sizeof(struct queue_item));
  sem_init(&entry->semaphore, 0, 0);
  entry->entries.sle_next = NULL;
  entry->left_state = RUNNING;

  pthread_mutex_lock(&men_queue_mutex);

  if (is_vip(info))
    insert_at_end_of_slist(&men_restroom_vip_queue, entry);
  else
    insert_at_end_of_slist(&men_restroom_queue, entry);

  pthread_mutex_unlock(&men_queue_mutex);

  // Mensagem de entrada de user na casa de banho dos menes
  char buffer[MAX_MESSAGE_BUFFER_SIZE];
  snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE - 1, "%d", info->i);
  thread_send_message_to_socket(info->socket_monitor, ENWCH, buffer);

  // Esperar pela vez do utilizador para entrar na atração
  sem_wait(&entry->semaphore);

  // Lock duplo para que só um utilizador esteja a sair da casa de banho num
  // determinado instante
  sem_post(&user_done_men_sem);

  // Todos os 3 tipos de mensagens que podem ser enviadas, vão enviar o id
  snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE - 1, "%d", info->i);

  MessageType type;
  bool accident = entry->left_state == ACCIDENT;

  // Determinar o que fazer assim que o utilizador saia da atração
  if (entry->left_state == RUNNING)
    type = EXWCH; // User usou e saiu da casa de banho
  else if (entry->left_state == QUIT)
    type = DESIS; // User desistiu da fila de espera
  else if (entry->left_state == ACCIDENT)
    type = ACCID; // User teve um acidente e tem que sair do parque

  pthread_mutex_lock(&men_queue_mutex);
  // Enviar mensagem para o monitor
  thread_send_message_to_socket(info->socket_monitor, type, buffer);

  sem_destroy(&entry->semaphore);

  pthread_mutex_unlock(&men_queue_mutex);
  sem_wait(&worker_done_men_sem);

  free(entry);
  return accident;
}

//-----------------------------------------------------------------
//               FIM CASA DE BANHO DE HOMENS
//-----------------------------------------------------------------

//-----------------------------------------------------------------
//                 CASA DE BANHO DE MULHERES
//-----------------------------------------------------------------

// Lista de espera para a casa de banho dos deficientes
struct queue_head women_restroom_queue =
    SLIST_HEAD_INITIALIZER(women_restroom_queue);
struct queue_head women_restroom_vip_queue =
    SLIST_HEAD_INITIALIZER(women_restroom_vip_queue);

// Controla que só um user pode estar a entrar ou sair da casa de banho
pthread_mutex_t women_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

// Exclusão mútua, variável number_of_people_in_women_bathrooms
pthread_mutex_t number_people_in_women_bathroom_mutex =
    PTHREAD_MUTEX_INITIALIZER;

// Controla quantas pessoas podem estar dentro das casas de banho dos howomens
int number_of_people_in_women_bahrooms = 0;

// Neste caso, duas podem estar, pois há duas casas de banho
#define LIMIT_PEOPLE_IN_WOMEN_BATHROOMS 2

// Age como um trinco duplo, para que um utilizador possa
// sair da casa de banho sem ser interrompido
sem_t user_done_women_sem, worker_done_women_sem;

void women_bathroom_worker_entry_point() {
  sem_init(&user_done_women_sem, 0, 0);
  sem_init(&worker_done_women_sem, 0, 0);

  SLIST_INIT(&women_restroom_queue);
  SLIST_INIT(&women_restroom_vip_queue);

  // TODO remove busy waiting
  while (parque_aberto) {

    pthread_mutex_lock(&women_queue_mutex);
    // Não libertar pessoas da fila se existem mais que duas pessoas nas casas
    // de banho
    if (number_of_people_in_women_bahrooms >= LIMIT_PEOPLE_IN_WOMEN_BATHROOMS) {
      continue;
    }

    // Tentar libertar primeiro a lista dos vips antes de deixar os outros
    // passarem
    struct queue_item *head = NULL;

    if (!SLIST_EMPTY(&women_restroom_vip_queue)) {
      head = SLIST_FIRST(&women_restroom_vip_queue);
      SLIST_REMOVE_HEAD(&women_restroom_vip_queue, entries);
    } else if (!SLIST_EMPTY(&women_restroom_queue)) {
      head = SLIST_FIRST(&women_restroom_queue);
      SLIST_REMOVE_HEAD(&women_restroom_queue, entries);
    }

    pthread_mutex_unlock(&women_queue_mutex);

    if (head != NULL) {

      // Chance de 1 em 1000 de haver um acidente
      if (rand() % 1000 == 0) {
        head->left_state = ACCIDENT;
      }

      // Fazer um user entrar na casa de banho
      number_of_people_in_women_bahrooms++;

      sem_post(&head->semaphore);

      sem_wait(&user_done_women_sem);
      // User a sair da casa de banho

      number_of_people_in_women_bahrooms--;
      // Sempre que um utilizador sai na casa de banho, rolar uma chance de os
      // outros desistirem
      // TODO Desistências
      struct queue_item *user = NULL;

      // Rolar uma chance para todos os utilizadores desistirem das filas de
      // espera da WC
      sem_post(&worker_done_women_sem);

      pthread_mutex_lock(&women_queue_mutex);
      if (!(SLIST_EMPTY(&women_restroom_queue))) {
        SLIST_FOREACH(user, &women_restroom_queue, entries) {
          if (rand() % 20 == 0 && user != NULL) {
            SLIST_REMOVE(&women_restroom_queue, user, queue_item, entries);
            user->left_state = QUIT;
            sem_post(&user->semaphore);

            sem_wait(&user_done_women_sem);
            sem_post(&worker_done_women_sem);
          }
        }

        user = NULL;
        if (!(SLIST_EMPTY(&women_restroom_vip_queue))) {
          SLIST_FOREACH(user, &women_restroom_vip_queue, entries) {
            if (rand() % 20 == 0 && user != NULL) {
              SLIST_REMOVE(&women_restroom_vip_queue, user, queue_item,
                           entries);
              user->left_state = QUIT;
              sem_post(&user->semaphore);

              sem_wait(&user_done_women_sem);
              sem_post(&worker_done_women_sem);
            }
          }
        }
        pthread_mutex_unlock(&women_queue_mutex);
      }
    }
  };

  struct queue_item *user = SLIST_FIRST(&women_restroom_queue);

  // Libertar todos os utilizadores da lista de espera normal
  while (user != NULL) {
    pthread_mutex_lock(&women_queue_mutex);
    user->left_state = QUIT;
    sem_post(&user->semaphore);

    sem_wait(&user_done_women_sem);
    sem_post(&worker_done_women_sem);
    pthread_mutex_unlock(&women_queue_mutex);

    user = user->entries.sle_next;
  };

  // Libertar todos os utilizadores da lista de espera vip
  user = SLIST_FIRST(&women_restroom_vip_queue);
  while (user != NULL) {
    pthread_mutex_lock(&women_queue_mutex);
    user->left_state = QUIT;
    sem_post(&user->semaphore);

    sem_wait(&user_done_women_sem);
    sem_post(&worker_done_women_sem);
    pthread_mutex_unlock(&women_queue_mutex);
  };
}

bool women_wc(user_info *info) {

  // Criar entrada deste user na lista de espera da casa de banho
  struct queue_item *entry = malloc(sizeof(struct queue_item));
  sem_init(&entry->semaphore, 0, 0);
  entry->entries.sle_next = NULL;
  entry->left_state = RUNNING;

  pthread_mutex_lock(&women_queue_mutex);

  if (is_vip(info))
    insert_at_end_of_slist(&women_restroom_vip_queue, entry);
  else
    insert_at_end_of_slist(&women_restroom_queue, entry);

  pthread_mutex_unlock(&women_queue_mutex);

  // Esperar pela vez do utilizador para entrar na atração
  sem_wait(&entry->semaphore);

  // Mensagem de entrada de user na casa de banho dos menes
  char buffer[MAX_MESSAGE_BUFFER_SIZE];
  snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE - 1, "%d", info->i);
  thread_send_message_to_socket(info->socket_monitor, ENWCW, buffer);

  // Lock duplo para que só um utilizador esteja a sair da casa de banho num
  // determinado instante
  sem_post(&user_done_women_sem);

  // Todos os 3 tipos de mensagens que podem ser enviadas, vão enviar o id
  snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE - 1, "%d", info->i);

  MessageType type;
  bool accident = entry->left_state == ACCIDENT;

  // Determinar o que fazer assim que o utilizador saia da atração
  if (entry->left_state == RUNNING)
    type = EXWCW; // User usou e saiu da casa de banho
  else if (entry->left_state == QUIT)
    type = DESIS; // User desistiu da fila de espera
  else if (entry->left_state == ACCIDENT)
    type = ACCID; // User teve um acidente e tem que sair do parque

  pthread_mutex_lock(&men_queue_mutex);
  // Enviar mensagem para o monitor
  thread_send_message_to_socket(info->socket_monitor, type, buffer);

  sem_destroy(&entry->semaphore);
  pthread_mutex_unlock(&men_queue_mutex);

  sem_wait(&worker_done_women_sem);

  free(entry);
  return accident;
}

//-----------------------------------------------------------------
//               FIM CASA DE BANHO DE HOMENS
//-----------------------------------------------------------------
