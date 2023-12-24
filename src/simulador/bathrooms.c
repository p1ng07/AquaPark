#include "bathrooms.h"
#include "../common/common.h"
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/queue.h>

extern bool parque_aberto;

// Lista de espera para a casa de banho dos deficientes
SLIST_HEAD(deficient_queue_head, queue_item)
deficient_restroom_queue = SLIST_HEAD_INITIALIZER(deficient_restroom_queue);

// Controla que só um user pode estar a entrar ou sair da casa de banho
pthread_mutex_t deficient_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

 // Age como um trinco duplo, para que um utilizador possa
// sair da casa de banho sem ser interrompido
sem_t user_done_sem, worker_done_sem;

int slist_length(struct deficient_queue_head* head){

  struct queue_item* curr = SLIST_FIRST(head);

  int length = 0;

  while (SLIST_NEXT(curr, entries) != NULL){
    curr = SLIST_NEXT(curr, entries);
    length++;
  }

  return length;
}

void insert_at_end_of_slist(struct deficient_queue_head* head, struct queue_item* entry){

  struct queue_item* curr = SLIST_FIRST(head);

  if (curr == NULL) {
    // Lista está vazia, inserir nova head
    SLIST_INSERT_HEAD(head, entry, entries);
    return;
  }

  while (SLIST_NEXT(curr, entries) != NULL){
    curr = SLIST_NEXT(curr, entries);
  }

  SLIST_INSERT_AFTER(curr, entry, entries);
}

bool enter_bathrooms(user_info *info) {

  // Determina se houve um acidente
  bool accident = false;

  if (info->deficient) {

    pthread_mutex_lock(&deficient_queue_mutex);

    // Criar entrada deste user na lista de espera da casa de banho
    struct queue_item *entry = malloc(sizeof(struct queue_item));
    sem_init(&entry->semaphore, 0, 0);
    entry->entries.sle_next = NULL;
    entry->left_state = RUNNING;

    insert_at_end_of_slist(&deficient_restroom_queue, entry);

    // Mensagem de entrada de user na casa de banho
    char buffer[MAX_MESSAGE_BUFFER_SIZE];
    snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE - 1, "%d", info->i);

    thread_send_message_to_socket(info->socket_monitor, ENWCD, buffer);

    pthread_mutex_unlock(&deficient_queue_mutex);

    // Esperar pela vez do utilizador para entrar no parque
    sem_wait(&entry->semaphore);

    // Lock duplo para que só um utilizador esteja a sair da casa de banho num
    // determinado instante
    sem_post(&user_done_sem);

    // Determinar o que fazer assim que o utilizador saia da atração
    switch (entry->left_state) {
    case RUNNING: {
      // User usou e saiu da casa de banho
      char buffer[MAX_MESSAGE_BUFFER_SIZE];
      snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE - 1, "%d", info->i);

      thread_send_message_to_socket(info->socket_monitor, EXWCD, buffer);
    } break;
    case QUIT: {
      // User desistiu da fila de espera
      char buffer[MAX_MESSAGE_BUFFER_SIZE];
      snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE - 1, "%d", info->i);

      thread_send_message_to_socket(info->socket_monitor, DESIS, buffer);
    } break;
    case ACCIDENT: {
      // TODO Isto talvez seja uma má decisão, tentar tirar isto daqui, nã̀o perdendo
      // o "realismo" de entrar primeiro numa casa de banho, depois ter um
      // acidente
      // User teve um acidente e tem que sair do parque
      char buffer[MAX_MESSAGE_BUFFER_SIZE];
      snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE - 1, "%d", info->i);
      thread_send_message_to_socket(info->socket_monitor, ACCID, buffer);
      accident = true;
    } break;
    default:
      assert(false);
      break;
    }
    sem_destroy(&entry->semaphore);
    free(entry);
    sem_wait(&worker_done_sem);

  } else if (info->is_man) {
    // TODO Casas de banho duplas de homens
    // Tentar usar ao máximo o que já foi feito
  } else if (!info->is_man) {
    // Casas de banho duplas de mulheres
  }

  return accident;
}

void disabled_bathroom_worker_entry_point() {
  sem_init(&user_done_sem, 0, 0);
  sem_init(&worker_done_sem, 0, 0);

  SLIST_INIT(&deficient_restroom_queue);

  // TODO remove busy waiting
  while (parque_aberto) {
    pthread_mutex_lock(&deficient_queue_mutex);

    // Tentar libertar a head da lista de utilizadores há espera
    struct queue_item *head = SLIST_FIRST(&deficient_restroom_queue);

    if (head != NULL) {

      // TODO Adicionar acidentes aqui
      // Ideia: Meter uma variável de estado na info do user, {RUNNING, QUIT,
      // ACCIDENT} Que diz o que o user deve fazer assim que sair de uma
      // atração.
      SLIST_REMOVE_HEAD(&deficient_restroom_queue, entries);
      sem_post(&head->semaphore);
      
      sem_wait(&user_done_sem);
      sem_post(&worker_done_sem);

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

	    sem_wait(&user_done_sem);
	    sem_post(&worker_done_sem);
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
    
    sem_wait(&user_done_sem);
    sem_post(&worker_done_sem);
  }

  printf("Worker thread de casas de batho saiu");
}
