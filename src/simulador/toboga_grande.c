#include "toboga_grande.h"

#include "../common/common.h"
#include "slist.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern bool parque_aberto;

// Fila de espera normal
struct queue_head tobogan_grande_queue =
    SLIST_HEAD_INITIALIZER(tobogan_grande_queue);

// Fila de espera vip
struct queue_head tobogan_grande_vip_queue =
    SLIST_HEAD_INITIALIZER(tobogan_grande_vip_queue);

// Exclusão mútua para as filas de espera 
pthread_mutex_t tobogan_grande_mutex = PTHREAD_MUTEX_INITIALIZER;

// Age como um trinco duplo, para que um utilizador possa
// sair da casa de banho sem ser interrompido
sem_t user_done_tobogan_grande_sem, worker_done_tobogan_grande_sem;

void tobogan_grande_worker_entry_point(){
  sem_init(&user_done_tobogan_grande_sem, 0, 0);
  sem_init(&worker_done_tobogan_grande_sem, 0, 0);

  SLIST_INIT(&tobogan_grande_queue);
  SLIST_INIT(&tobogan_grande_vip_queue);

  while(parque_aberto){

    // Esperar que haja alguém na fila

    pthread_mutex_lock(&tobogan_grande_mutex);

    // Tentar libertar dois users das filas de espera
    struct queue_item *head = NULL;

    if (!SLIST_EMPTY(&tobogan_grande_vip_queue)) {
      head = SLIST_FIRST(&tobogan_grande_vip_queue);
      SLIST_REMOVE_HEAD(&tobogan_grande_vip_queue, entries);
    } else if (!SLIST_EMPTY(&tobogan_grande_queue)) {
      head = SLIST_FIRST(&tobogan_grande_queue);
      SLIST_REMOVE_HEAD(&tobogan_grande_queue, entries);
    }

    struct queue_item *tail = NULL;
    if (!SLIST_EMPTY(&tobogan_grande_vip_queue)) {
      tail = SLIST_FIRST(&tobogan_grande_vip_queue);
      SLIST_REMOVE_HEAD(&tobogan_grande_vip_queue, entries);
    } else if (!SLIST_EMPTY(&tobogan_grande_queue)) {
      tail = SLIST_FIRST(&tobogan_grande_queue);
      SLIST_REMOVE_HEAD(&tobogan_grande_queue, entries);
    }

    /* pthread_mutex_unlock(&tobogan_grande_mutex); */

    if (head != NULL) {

      // Chance de 1 em 1000 de haver um acidente
      if (rand() % 1000 == 0) {
        head->left_state = ACCIDENT;
      }

      sem_post(&head->semaphore);

      // Acionar segundo utilizador a ter um acidentte, ou entrar na atração
      if (tail) {
        if (rand() % 1000 == 0) {
          tail->left_state = ACCIDENT;
        }

        sem_post(&tail->semaphore);
      }

      // Esperar que primeiro utilizador entre na atração
      sem_wait(&user_done_tobogan_grande_sem);

      if (tail) {
        // Se existir um segundo utilizador, esperar que este entre na atração
        sem_wait(&user_done_tobogan_grande_sem);
      }

      sem_post(&worker_done_tobogan_grande_sem);

      if (tail) {
        // Se existir um segundo utilizador, esperar que este entre na atração
        sem_post(&worker_done_tobogan_grande_sem);
      }

      //----------------------DESISTÊNCIAS-----------------------
      // Rola uma chance para que, individualmente, todos os utilizadores
      // desistam das filas de espera
      // Sempre que alguém entra na atração, rolar uma chance para que todos os
      // utilizadores, individualmente, desistam das suas filas de espera
      /* pthread_mutex_lock(&tobogan_grande_mutex); */
      struct queue_item *user = NULL;
      if (!(SLIST_EMPTY(&tobogan_grande_queue))) {
        SLIST_FOREACH(user, &tobogan_grande_queue, entries) {
          if (rand() % 20 == 0 && user != NULL) {
            SLIST_REMOVE(&tobogan_grande_queue, user, queue_item, entries);
            user->left_state = QUIT;
            sem_post(&user->semaphore);

            sem_wait(&user_done_tobogan_grande_sem);
            sem_post(&worker_done_tobogan_grande_sem);
          }
        }
      }

      user = NULL;
      if (!(SLIST_EMPTY(&tobogan_grande_vip_queue))) {
        SLIST_FOREACH(user, &tobogan_grande_vip_queue, entries) {
          if (rand() % 20 == 0 && user != NULL) {
            SLIST_REMOVE(&tobogan_grande_vip_queue, user, queue_item, entries);
            user->left_state = QUIT;
            sem_post(&user->semaphore);

            sem_wait(&user_done_tobogan_grande_sem);
            sem_post(&worker_done_tobogan_grande_sem);
          }
        }
      }
      //----------------- FIM DESISTÊNCIAS ------------------

      // Esperar 0.5 milisegundos para que haja mais que uma pessoa nas filas de
      // espera
      usleep(500);
    }
    pthread_mutex_unlock(&tobogan_grande_mutex);
  }

  // Libertar todos os utilizadores na fila de espera quando o parque fecha
  struct queue_item *user = NULL;

  while (user) {
    user->left_state = QUIT;
    sem_post(&user->semaphore);

    sem_wait(&user_done_tobogan_grande_sem);
    sem_post(&worker_done_tobogan_grande_sem);

    user = user->entries.sle_next;
  };

  user = SLIST_FIRST(&tobogan_grande_vip_queue);
  while (user) {
    user->left_state = QUIT;
    sem_post(&user->semaphore);

    sem_wait(&user_done_tobogan_grande_sem);
    sem_post(&worker_done_tobogan_grande_sem);

    user = user->entries.sle_next;
  };
}
  
bool tobogan_grande(user_info *info){
  
  pthread_mutex_lock(&tobogan_grande_mutex);

  // Criar entrada deste user fila de espera
  struct queue_item *entry = malloc(sizeof(struct queue_item));
  sem_init(&entry->semaphore, 0, 0);
  entry->entries.sle_next = NULL;
  entry->left_state = RUNNING;

  if (is_vip(info))
    insert_at_end_of_slist(&tobogan_grande_vip_queue, entry);
  else
    insert_at_end_of_slist(&tobogan_grande_queue, entry);

  // Mensagem de entrada de user na fila de espera do tobogan grande
  char buffer[MAX_MESSAGE_BUFFER_SIZE];
  snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE - 1, "%d,%d", info->i, (is_vip(info)? 1 : 0));
  thread_send_message_to_socket(info->socket_monitor, ENTBG, buffer);

  pthread_mutex_unlock(&tobogan_grande_mutex);

  // Esperar pela vez do utilizador para entrar na atração
  sem_wait(&entry->semaphore);

  sem_post(&user_done_tobogan_grande_sem);

  // Todos os 3 tipos de mensagens que podem ser enviadas, vão enviar o id
  snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE - 1, "%d,%d", info->i, (is_vip(info)? 1 : 0));

  MessageType type;
  bool accident = entry->left_state == ACCIDENT;

  // Determinar o que fazer assim que o utilizador saia da atração
  if (entry->left_state == RUNNING)
    type = EXTBG; // User usou e saiu do tobogan grande
  else if (entry->left_state == QUIT)
    type = DESIS; // User desistiu da fila de espera
  else if (entry->left_state == ACCIDENT)
    type = ACCID; // User teve um acidente e tem que sair do parque

  // Enviar mensagem para o monitor
  thread_send_message_to_socket(info->socket_monitor, type, buffer);

  sem_destroy(&entry->semaphore);

  sem_wait(&worker_done_tobogan_grande_sem);

  free(entry);
  return accident;
}
