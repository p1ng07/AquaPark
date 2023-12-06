#include "user.h"
#include "../common/common.h"
#include "../common/communication.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

// Lock usado para que só uma thread esteja a enviar mensagens para o monitor
pthread_mutex_t communication_lock = PTHREAD_MUTEX_INITIALIZER;

void thread_send_message_to_socket(int *socket, MessageType type,
			    char *message) {
  pthread_mutex_lock(&communication_lock);
  send_message_to_socket(socket, type, message);
  pthread_mutex_unlock(&communication_lock);
}


// Entry point dos users
// A informação recebida deve ser desalocada antes da terminação da thread
void user_entry_point(user_entry_point_info* info){
  char buffer[MAX_MESSAGE_BUFFER_SIZE];
  snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE -1, "User %i criado", info->i);

  thread_send_message_to_socket(info->socket_monitor, EVENT, buffer);

  // Fazer uma ação para simular um evento do utilizador
  snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE -1, "Utilizador %i fez uma ação.", info->i);
  thread_send_message_to_socket(info->socket_monitor, MESNG, buffer);

  free(info);
}

