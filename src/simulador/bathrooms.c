#include "bathrooms.h"
#include "slist.h"
#include "../common/common.h"
#include "user.h"
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
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
  } else if (!info->is_man) {
    // Casas de banho duplas de mulheres
  }


  return accident;
}

//-----------------------------------------------------------------
//                 CASA DE BANHO DE DEFICIENTES
//-----------------------------------------------------------------

// Lista de espera para a casa de banho dos deficientes
struct queue_head deficient_restroom_queue = SLIST_HEAD_INITIALIZER(deficient_restroom_queue);

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
      if (rand() % 1000 == 0){
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
          if (rand() % 20 == 0 && slist_length(&deficient_restroom_queue) > 1) {
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

  }

  struct queue_item *user;

  // Libertar todos os utilizadores na fila de espera quando o parque fecha
  SLIST_FOREACH(user, &deficient_restroom_queue, entries) {
    user->left_state = QUIT;
    sem_post(&user->semaphore);
    
    sem_wait(&user_done_def_sem);
    sem_post(&worker_done_def_sem);
  }
}

bool disabled_wc(user_info* info){

  // Criar entrada deste user na lista de espera da casa de banho
  struct queue_item *entry = malloc(sizeof(struct queue_item));
  sem_init(&entry->semaphore, 0, 0);
  entry->entries.sle_next = NULL;
  entry->left_state = RUNNING;

  pthread_mutex_lock(&deficient_queue_mutex);

  insert_at_end_of_slist(&deficient_restroom_queue, entry);

  // Mensagem de entrada de user na casa de banho dos deficientes
  char buffer[MAX_MESSAGE_BUFFER_SIZE];
  snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE - 1, "%d", info->i);
  thread_send_message_to_socket(info->socket_monitor, ENWCD, buffer);

  pthread_mutex_unlock(&deficient_queue_mutex);

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
