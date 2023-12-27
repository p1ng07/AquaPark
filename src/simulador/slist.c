#include "slist.h"

int slist_length(struct queue_head* head){

  struct queue_item* curr = SLIST_FIRST(head);

  if(curr == NULL)
    return 0;

  int length = 0;

  while (SLIST_NEXT(curr, entries) != NULL) {
    curr = SLIST_NEXT(curr, entries);
    length++;
  }

  return length;
}

void insert_at_end_of_slist(struct queue_head* head, struct queue_item* entry){

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
