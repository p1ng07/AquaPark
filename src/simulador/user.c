#include "user.h"
#include "../common/common.h"
#include "../common/communication.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

// Lock usado para que só uma thread esteja a enviar mensagens para o monitor
pthread_mutex_t communication_lock = PTHREAD_MUTEX_INITIALIZER;

// Entry point dos users
// A informação recebida deve ser desalocada antes da terminação da thread
void user_entry_point(user_entry_point_info* info){

  char buffer[MAX_MESSAGE_BUFFER_SIZE];
  snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE, "Thread criada: %i", info->i);
  /* send_string_to_monitor(info->socket_monitor, buffer, EVENT); */


  // Fazer uma ação para simular um evento do utilizador
  send_string_to_monitor(info->socket_monitor, EVENT, "Utilizador %i fez uma ação.", info->i);

  free(info);
}

