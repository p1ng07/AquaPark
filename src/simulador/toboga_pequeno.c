
#include "slist.h"
#include "../common/common.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

extern bool parque_aberto;


// Lista de espera para a casa de banho dos deficientes
struct queue_head tobogan_pequeno_queue =
    SLIST_HEAD_INITIALIZER(tobogan_pequeno_queue);
struct queue_head tobogan_pequeno_vip_queue =
    SLIST_HEAD_INITIALIZER(tobogan_pequeno_vip_queue);

// Controla que só um user pode estar a entrar ou sair da casa de banho
pthread_mutex_t tobogan_pequeno_mutex = PTHREAD_MUTEX_INITIALIZER;

// Age como um trinco duplo, para que um utilizador possa
// sair da casa de banho sem ser interrompido
sem_t user_done_tobogan_pequeno_sem, worker_done_tobogan_pequeno_sem;

void tobogan_pequeno_worker_entry_point() {
  sem_init(&user_done_tobogan_pequeno_sem, 0, 0);
  sem_init(&worker_done_tobogan_pequeno_sem, 0, 0);

  SLIST_INIT(&tobogan_pequeno_queue);
  SLIST_INIT(&tobogan_pequeno_vip_queue);

  // TODO remove busy waiting
  while (parque_aberto) {

    pthread_mutex_lock(&tobogan_pequeno_mutex);

    // Tentar libertar primeiro a lista dos vips antes de deixar os outros
    // passarem
    struct queue_item *head = NULL;

    if (!SLIST_EMPTY(&tobogan_pequeno_vip_queue)) {
      head = SLIST_FIRST(&tobogan_pequeno_vip_queue);
      SLIST_REMOVE_HEAD(&tobogan_pequeno_vip_queue, entries);
    } else if (!SLIST_EMPTY(&tobogan_pequeno_queue)) {
      head = SLIST_FIRST(&tobogan_pequeno_queue);
      SLIST_REMOVE_HEAD(&tobogan_pequeno_queue, entries);
    }
    pthread_mutex_unlock(&tobogan_pequeno_mutex);

    if (head != NULL) {

      // Chance de 1 em 1000 de haver um acidente
      if (rand() % 1000 == 0) {
        head->left_state = ACCIDENT;
      }

      sem_post(&head->semaphore);

      sem_wait(&user_done_tobogan_pequeno_sem);
      // User a sair da casa de banho


      sem_post(&worker_done_tobogan_pequeno_sem);

      // Rola uma chance para que, individualmente, todos os utilizadores
      // desistam das filas de espera
      // Sempre que um utilizador sai na casa de banho, rolar uma chance de os
      // outros desistirem
      struct queue_item *user = NULL;
      pthread_mutex_lock(&tobogan_pequeno_mutex);
      if (!(SLIST_EMPTY(&tobogan_pequeno_queue))){
        SLIST_FOREACH(user, &tobogan_pequeno_queue, entries) {
          if (rand() % 20 == 0 && user != NULL) {
            SLIST_REMOVE(&tobogan_pequeno_queue, user, queue_item, entries);
            user->left_state = QUIT;
            sem_post(&user->semaphore);

            sem_wait(&user_done_tobogan_pequeno_sem);
            sem_post(&worker_done_tobogan_pequeno_sem);
          }
        }
      }

      user = NULL;
      if (!(SLIST_EMPTY(&tobogan_pequeno_vip_queue))) {
	SLIST_FOREACH(user, &tobogan_pequeno_vip_queue, entries) {
	  if (rand() % 20 == 0 && user != NULL) {
	    SLIST_REMOVE(&tobogan_pequeno_vip_queue, user, queue_item, entries);
            user->left_state = QUIT;
            sem_post(&user->semaphore);

            sem_wait(&user_done_tobogan_pequeno_sem);
            sem_post(&worker_done_tobogan_pequeno_sem);
          }
        }
      }
      pthread_mutex_unlock(&tobogan_pequeno_mutex);
    }
  };

  struct queue_item *user = NULL;

  // Libertar todos os utilizadores na fila de espera quando o parque fecha
  while (user) {
    user->left_state = QUIT;
    sem_post(&user->semaphore);

    sem_wait(&user_done_tobogan_pequeno_sem);
    sem_post(&worker_done_tobogan_pequeno_sem);

    user = user->entries.sle_next;
  };

  user = SLIST_FIRST(&tobogan_pequeno_vip_queue);
  while (user) {
    user->left_state = QUIT;
    sem_post(&user->semaphore);

    sem_wait(&user_done_tobogan_pequeno_sem);
    sem_post(&worker_done_tobogan_pequeno_sem);

    user = user->entries.sle_next;
  };
}

bool tobogan_pequeno(user_info *info) {

  // Criar entrada deste user na lista de espera da casa de banho
  struct queue_item *entry = malloc(sizeof(struct queue_item));
  sem_init(&entry->semaphore, 0, 0);
  entry->entries.sle_next = NULL;
  entry->left_state = RUNNING;

  pthread_mutex_lock(&tobogan_pequeno_mutex);

  if (is_vip(info))
    insert_at_end_of_slist(&tobogan_pequeno_vip_queue, entry);
  else
    insert_at_end_of_slist(&tobogan_pequeno_queue, entry);


  // Mensagem de entrada de user na casa de banho dos menes
  char buffer[MAX_MESSAGE_BUFFER_SIZE];
  snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE - 1, "%d,%d", info->i, (is_vip(info)? 1 : 0));
  thread_send_message_to_socket(info->socket_monitor, ENTBP, buffer);

  pthread_mutex_unlock(&tobogan_pequeno_mutex);

  // Esperar pela vez do utilizador para entrar na atração
  sem_wait(&entry->semaphore);

  // Lock duplo para que só um utilizador esteja a sair do tobogan
  // determinado instante
  sem_post(&user_done_tobogan_pequeno_sem);

  pthread_mutex_lock(&tobogan_pequeno_mutex);
  // Todos os 3 tipos de mensagens que podem ser enviadas, vão enviar o id
  snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE - 1, "%d,%d", info->i, (is_vip(info)? 1 : 0));

  MessageType type;
  bool accident = entry->left_state == ACCIDENT;

  // Determinar o que fazer assim que o utilizador saia da atração
  if (entry->left_state == RUNNING)
    type = EXTBP; // User usou e saiu da casa de banho
  else if (entry->left_state == QUIT)
    type = DESIS; // User desistiu da fila de espera
  else if (entry->left_state == ACCIDENT)
    type = ACCID; // User teve um acidente e tem que sair do parque

  // Enviar mensagem para o monitor
  thread_send_message_to_socket(info->socket_monitor, type, buffer);

  sem_destroy(&entry->semaphore);

  pthread_mutex_unlock(&tobogan_pequeno_mutex);
  sem_wait(&worker_done_tobogan_pequeno_sem);

  free(entry);
  return accident;
}
