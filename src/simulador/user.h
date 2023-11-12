#ifndef USER_H
#define USER_H

#include <pthread.h>
#include <bits/pthreadtypes.h>
#include "../common/communication.h"

typedef struct {
  int *socket_monitor;
  int i;
} user_entry_point_info;

void user_entry_point(user_entry_point_info *info);

// Manda uma mensagem do tipo type para o monitor, a mensagem tem de ter
// MAX_MESSAGE_SIZE - 1 de tamanho
#define send_string_to_monitor(socket_monitor, type, ...)                      \
  do {                                                                         \
    char buffer[MAX_MESSAGE_BUFFER_SIZE];                                      \
    snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE - 1, __VA_ARGS__);                \
    char identificador[MAX_MESSAGE_BUFFER_SIZE];                               \
    pthread_mutex_lock(&communication_lock);                                   \
    for (int i = 0; i < 5; i++)                                                \
      identificador[i] = string_from_com_type(type)[i];                        \
    send(*socket_monitor,                                                      \
	 strncat(identificador, buffer, MAX_MESSAGE_BUFFER_SIZE - 1),          \
	 MAX_MESSAGE_BUFFER_SIZE, 0);                                          \
    pthread_mutex_unlock(&communication_lock);                                 \
  } while (0)

#endif
