#ifndef BATHROOMS_H
#define BATHROOMS_H

#include "user.h"
#include <semaphore.h>
#include <sys/queue.h>

struct queue_item {
  sem_t semaphore;
  int i;
  bool quit; // Indica se um utilizador deve desistir da fila de espera ou não
  SLIST_ENTRY(queue_item) entries;
};

void enter_bathrooms(user_info *info);

void disabled_bathroom_worker_entry_point();

#endif
