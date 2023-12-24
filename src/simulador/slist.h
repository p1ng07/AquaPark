#ifndef SLIST_H
#define SLIST_H

#include "bathrooms.h"
#include <semaphore.h>

// Item que representa uma utilizador numa fila de espera fifo
struct queue_item {
  sem_t semaphore;       // Semáforo que faz o utilizador esperar a sua vez
  user_state left_state; // Esta variável indica o que o utilizador
			 // deve fazer assim que sair de uma atração
  SLIST_ENTRY(queue_item)
  entries; // Entrada da lista ligada que representa a fila de espera
};

struct queue_head {
  struct queue_item *slh_first; /* first element */
};

int slist_length(struct queue_head* head);

void insert_at_end_of_slist(struct queue_head* head, struct queue_item* entry);

#endif
