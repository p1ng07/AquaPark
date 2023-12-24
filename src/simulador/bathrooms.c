#include "bathrooms.h"
#include "../common/common.h"
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

void enter_bathrooms(user_info *info) {
  if (info->deficient) {

    pthread_mutex_lock(&deficient_queue_mutex);

    struct queue_item *entry = malloc(sizeof(struct queue_item));

    entry->quit = false;
    entry->i = info->i;
    sem_init(&entry->semaphore, 0, 0);
    entry->entries.sle_next = NULL;

    insert_at_end_of_slist(&deficient_restroom_queue, entry);

    // TODO add a message to monitor here
    printf("Utilizador %d entrou na casa de banho\n", info->i);

    struct queue_item *it = NULL;
    printf("[");
    SLIST_FOREACH(it, &deficient_restroom_queue, entries) {
      printf("%d,", it->i);
    }
    printf("]\n");

    pthread_mutex_unlock(&deficient_queue_mutex);

    // Esperar pela vez do utilizador para entrar no parque
    sem_wait(&entry->semaphore);

    sem_post(&user_done_sem);
    if (entry->quit){
      // User desistiu da fila de espera
      printf("Utilizador %d desistiu\n", entry->i);
      printf("[");
      struct queue_item *it = NULL;
      SLIST_FOREACH(it, &deficient_restroom_queue, entries) {
	if (it) {
	  printf("%d,", it->i);
	}
      }
      printf("]\n");
    } else {
      // User usou e saiu da casa de banho
      printf("Utilizador %d saiu na casa de banho \n", info->i);

      printf("[");
      SLIST_FOREACH(it, &deficient_restroom_queue, entries) {
	printf("%d,", it->i);
      }
      printf("]\n");
    }
    sem_destroy(&entry->semaphore);
    free(entry);
    sem_wait(&worker_done_sem);


    return;
  } else if (info->is_man) {
    // Casas de banho duplas de homens
  } else if (!info->is_man) {
    // Casas de banho duplas de mulheres
  }
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
          if (rand() % 10 < 6 && slist_length(&deficient_restroom_queue) > 1) {
            SLIST_REMOVE(&deficient_restroom_queue, user, queue_item, entries);
            user->quit = true;
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
  pthread_mutex_lock(&deficient_queue_mutex);
  SLIST_FOREACH(user, &deficient_restroom_queue, entries) {
    user->quit = true;
    sem_post(&user->semaphore);
  }
  pthread_mutex_unlock(&deficient_queue_mutex);
}
