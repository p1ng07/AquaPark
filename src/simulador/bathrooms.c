#include "bathrooms.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/queue.h>

extern bool parque_aberto;

// Lista de espera para a casa de banho dos deficientes
SLIST_HEAD(deficient_queue_head, queue_item)
deficient_restroom_queue = SLIST_HEAD_INITIALIZER(deficient_restroom_queue);

pthread_mutex_t deficient_bathroom_use_lock = PTHREAD_MUTEX_INITIALIZER;
sem_t sem_user_done;
sem_t sem_worker_done;

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

    pthread_mutex_lock(&deficient_bathroom_use_lock);

    struct queue_item *entry = malloc(sizeof(struct queue_item));

    entry->quit = false;
    entry->i = info->i;
    sem_init(&entry->semaphore, 0, 0);
    entry->entries.sle_next = NULL;

    insert_at_end_of_slist(&deficient_restroom_queue, entry);

    printf("Utilizador %d chegou a casa de banho\n", info->i);
    struct queue_item *it = NULL;
    printf("[");
    SLIST_FOREACH(it, &deficient_restroom_queue, entries) {
      printf("%d,", it->i);
    }
    printf("]\n");

    pthread_mutex_unlock(&deficient_bathroom_use_lock);
    // Esperar pela vez do utilizador para entrar no parque
    sem_wait(&entry->semaphore);

    pthread_mutex_lock(&deficient_bathroom_use_lock);

    if (entry->quit) {
      // User foi libertado, mas deve desistir da atração
      sem_destroy(&entry->semaphore);
      free(entry);
      pthread_mutex_unlock(&deficient_bathroom_use_lock);
      return;
    }

    printf("Utilizador %d saiu da casa de banho\n", info->i);
    printf("[");
    SLIST_FOREACH(it, &deficient_restroom_queue, entries) {
      printf("%d,", it->i);
    }
    printf("]\n");
    
    pthread_mutex_unlock(&deficient_bathroom_use_lock);

    sem_wait(&sem_user_done);

    sem_destroy(&entry->semaphore);
    free(entry);
    return;
  } else if (info->is_man) {
    // Casas de banho duplas de homens
  } else if (!info->is_man) {
    // Casas de banho duplas de mulheres
  }
}

void disabled_bathroom_worker_entry_point() {

  sem_init(&sem_user_done, 0,0);
  sem_init(&sem_user_done, 0,0);

  SLIST_INIT(&deficient_restroom_queue);

  // TODO remove busy waiting
  while (parque_aberto) {
    pthread_mutex_lock(&deficient_bathroom_use_lock);

    // Tentar libertar a head da lista de utilizadores há espera
    struct queue_item *head = SLIST_FIRST(&deficient_restroom_queue);

    if (head != NULL) {

      sem_post(&head->semaphore);
      SLIST_REMOVE_HEAD(&deficient_restroom_queue, entries);
      
      // Sempre que um utilizador sai na casa de banho, rolar uma chance de os
      // outros desistirem

      struct queue_item *user = NULL;
      SLIST_FOREACH(user, &deficient_restroom_queue, entries) {
        if (user) {
          // TODO Adicionar chance de desistência a um parametro no ficheiro de
          // configuração
          /* if (rand() % 10 < 5) { */
          /*   printf("Utilizador %d desistiu\n", user->i); */
          /*   SLIST_REMOVE(&deficient_restroom_queue, user, queue_item, entries); */
          /*   user->quit = true; */
          /*   sem_post(&user->semaphore); */
	  /*   printf("["); */
	  /*   SLIST_FOREACH(user, &deficient_restroom_queue, entries) { */
	  /*     printf("%d,"user->i); */
          /*   } */
	  /*   printf("]"); */
          /* } */
        }
      }
      sem_post(&sem_user_done);
    }
    pthread_mutex_unlock(&deficient_bathroom_use_lock);

  }

  struct queue_item *user;

  // Libertar todos os utilizadores na fila de espera quando o parque fecha
  pthread_mutex_lock(&deficient_bathroom_use_lock);
  SLIST_FOREACH(user, &deficient_restroom_queue, entries) {
    user->quit = true;
    sem_post(&user->semaphore);
  }
  pthread_mutex_unlock(&deficient_bathroom_use_lock);
}
