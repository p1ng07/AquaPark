#include "user.h"
#include "../common/common.h"
#include "../common/communication.h"
#include "bathrooms.h"
#include <assert.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

extern bool parque_aberto;

extern unsigned long *global_user_thread_list;

// Entry point dos users
// A informação recebida deve ser desalocada antes da terminação da thread
void user_entry_point(user_info* info){
  char buffer[MAX_MESSAGE_BUFFER_SIZE];
  snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE -1, "%d", info->i);

  thread_send_message_to_socket(info->socket_monitor, ENTER, buffer);

  // TODO Handle if park is closed (simple queue)

  while(parque_aberto){
    // Fazendo coisas dentro do parque

    bool accident = try_enter_attractions(info);
    if (accident){
      snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE - 1, "%d", info->i);
      thread_send_message_to_socket(info->socket_monitor, ACCID, buffer);
      break;
    }
  };

  // Cleanup de sair do parque (libertar a entry deste user na lista global de threads no simulador.c, fazendo espaço para novos utilizadores)

  for (int i = 0; i < MAX_THREADS; i++){
    if (info->pthread_info == global_user_thread_list[i]){
      global_user_thread_list[i] = 0;
    }
  }

  // Sair do parque
  snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE -1, "%i", info->i);

  thread_send_message_to_socket(info->socket_monitor, EXITU, buffer);

  free(info);
}

bool try_enter_attractions(user_info *info) {
  // TODO adicionar diversões

  // Escolher um número de 0 a 5, no caso de um adulto, e de 0 a 6 no caso de uma criança
  int attraction = rand() % ((info->age > 12) ? 5 : 6);

  switch (attraction) {
  case 0: {
    // TODO Bar
  } break;
  case 1: {
    // TODO Piscina Grande
  } break;
  case 2: {
    // TODO Toboga grande
  } break;
  case 3: {
    // TODO Toboga pequena
  } break;
  case 4: {
    // TODO casas de banho
    return enter_bathrooms(info);
  } break;
  case 5: {
    // Piscina das crianças
  } break;
  default: {
    assert(0);
    break;
  }
  }

  // TODO Sempre que um user estiver numa atração, deve rolar uma chance de ele ter um acidente e ter de sair do parque, returnando true nesta função
  return false;
}

// Lock usado para que só uma thread esteja a enviar mensagens para o monitor
pthread_mutex_t communication_lock = PTHREAD_MUTEX_INITIALIZER;


void thread_send_message_to_socket(int *socket, MessageType type,
			    char *message) {
  pthread_mutex_lock(&communication_lock);
  send_message_to_socket(socket, type, message);
  pthread_mutex_unlock(&communication_lock);
}

bool is_vip(user_info* info){
  return info->deficient || info->age > 69;
}
