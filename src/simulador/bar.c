#include "bar.h"
#include "user.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>

// Capacidade corrente da bar
int capacity_bar = 0;

// Trinco para garantir exclusão mútua quando se acessa à capacidade
pthread_mutex_t trinco_bar = PTHREAD_MUTEX_INITIALIZER;

bool bar(user_info* info){
  pthread_mutex_lock(&trinco_bar);
    // Tentativa de entrada do user na bar

  char buffer[MAX_MESSAGE_BUFFER_SIZE];
  snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE-1, "%d", info->i);

  capacity_bar++;

  thread_send_message_to_socket(info->socket_monitor, ENBAR, buffer);
  
  pthread_mutex_unlock(&trinco_bar);
  
  pthread_mutex_lock(&trinco_bar);
  // User está dentro da bar, correr uma chance de acidente

  if(should_have_accident()){
    // User teve um acidente
    capacity_bar--;
    thread_send_message_to_socket(info->socket_monitor, ACCID_BAR, buffer);
    pthread_mutex_unlock(&trinco_bar);

    return true;
  }

  // User está a sair da bar, mandar mensagem de saída
  thread_send_message_to_socket(info->socket_monitor, ENBAR, buffer);
  capacity_bar--;
  pthread_mutex_unlock(&trinco_bar);

  return false;
}
