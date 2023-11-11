#include "user.h"
#include "../common/common.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

// Lock usado para que só uma thread esteja a enviar mensagens para o monitor
pthread_mutex_t communication_lock = PTHREAD_MUTEX_INITIALIZER;

void user_entry_point(user_entry_point_info* info){
  char buffer[MAX_MESSAGE_BUFFER_SIZE];
  snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE, "Thread criada: %i \n", info->i);

  send_string_to_monitor(info->socket_monitor, buffer);
}

void send_string_to_monitor(int* socket_monitor, char* mensagem){
  pthread_mutex_lock(&communication_lock);
  send(*socket_monitor, mensagem, MAX_MESSAGE_BUFFER_SIZE, 0);
  pthread_mutex_unlock(&communication_lock);
}
