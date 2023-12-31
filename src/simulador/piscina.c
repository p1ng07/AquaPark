#include "piscina.h"
#include "user.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>

// Capacidade corrente da piscina
int capacity = 0;

// Trinco para garantir exclusão mútua quando se acessa à capacidade
pthread_mutex_t trinco = PTHREAD_MUTEX_INITIALIZER;

bool piscina(user_info* info){
  pthread_mutex_lock(&trinco);
    // Tentativa de entrada do user na piscina

  char buffer[MAX_MESSAGE_BUFFER_SIZE];
  snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE-1, "%d", info->i);

  if(capacity + 1 >= MAX_CAPACITY_PISCINA){
    // Não há espaço para o user na piscina
    thread_send_message_to_socket(info->socket_monitor, FAIL_PIS, buffer);
    pthread_mutex_unlock(&trinco);
    return false;
  }

  capacity++;

  thread_send_message_to_socket(info->socket_monitor, ENPIS, buffer);
  
  pthread_mutex_unlock(&trinco);
  
  pthread_mutex_lock(&trinco);
  // User está dentro da piscina, correr uma chance de acidente

  if(should_have_accident()){
    // User teve um acidente
    capacity--;
    thread_send_message_to_socket(info->socket_monitor, ACCID_PIS, buffer);
    pthread_mutex_unlock(&trinco);

    return true;
  }

  // User está a sair da piscina, mandar mensagem de saída
  thread_send_message_to_socket(info->socket_monitor, EXPIS, buffer);
  capacity--;
  pthread_mutex_unlock(&trinco);

  return false;
}
