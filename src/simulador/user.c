#include "user.h"
#include "../common/common.h"
#include "../common/communication.h"
#include "bathrooms.h"
#include "bar.h"
#include "piscina.h"
#include "piscina_criancas.h"
#include "toboga_grande.h"
#include "toboga_pequeno.h"
#include <assert.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

extern bool parque_aberto;

extern pthread_mutex_t global_user_thread_list_mutex;

extern unsigned long *global_user_thread_list;

// Parâmetros que são carregados no início do simulação, antes do utilizador ser
// criado;
extern int
    have_accident_parameter; // Define a % chance (0 a 100) de haver um acidente

extern int quit_attraction_parameter; // Define a % chance (0 a 100) de um
                                      // user desistir de uma fila de espera

void exit_park(user_info* info){
  pthread_mutex_lock(&global_user_thread_list_mutex);
  for (int i = 0; i < MAX_THREADS; i++) {

    if (info->pthread_info == global_user_thread_list[i]){
      global_user_thread_list[i] = 0;
    }
  };
  pthread_mutex_unlock(&global_user_thread_list_mutex);

  // Sair do parque
  char buffer[MAX_MESSAGE_BUFFER_SIZE];
  snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE - 1, "%i", info->i);

  thread_send_message_to_socket(info->socket_monitor, EXITU, buffer);

  free(info);
}

// Entry point dos users
// A informação recebida deve ser desalocada antes da terminação da thread
void user_entry_point(user_info* info){

  pthread_cleanup_push((void*)exit_park, info);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  char buffer[MAX_MESSAGE_BUFFER_SIZE];
  snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE -1, "%d", info->i);

  thread_send_message_to_socket(info->socket_monitor, ENTER, buffer);

  while (1) {
    // Fazendo coisas dentro do parque
    bool accident = try_enter_attractions(info);

    if (accident)
      break;
  };

  // Cleanup de sair do parque (libertar a entry deste user na lista global de
  // threads no simulador.c, fazendo espaço para novos utilizadores)
  pthread_cleanup_pop(1);
}

bool try_enter_attractions(user_info *info) {
  // Escolher um número de 0 a 5, no caso de um adulto, e de 0 a 6 no caso de
  // uma criança
  int attraction = rand() % ((info->age > 12) ? 5 : 6);
  /* int attraction = 4; */

  switch (attraction) {
  case 0: {
    return bar(info);
  } break;
  case 1: {
    return piscina(info);
  } break;
  case 2: {
    return tobogan_grande(info);
  } break;
  case 3: {
    return tobogan_pequeno(info);
  } break;
  case 4: {
    return enter_bathrooms(info);
  } break;
  case 5: {
    // Piscina das crianças
    return piscina_criancas(info);
  } break;
  default: {
    assert(0);
    break;
  }
  }

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

bool inline should_have_accident(){
  srand(time(NULL));
  int random = rand() % 100;

  return random < have_accident_parameter;
}

bool inline should_quit_wait_queue(){
  srand(time(NULL));

  return (rand() % 100) < quit_attraction_parameter;
}
