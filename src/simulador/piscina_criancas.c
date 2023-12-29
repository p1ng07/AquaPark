#include "piscina_criancas.h"
#include "user.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>

// Capacidade corrente da piscina
int capacity_criancas = 0;

// Trinco_Piscina_Criancas para garantir exclusão mútua quando se acessa à capacidade
pthread_mutex_t trinco_piscina_criancas = PTHREAD_MUTEX_INITIALIZER;

bool piscina_criancas(user_info* info){
  pthread_mutex_lock(&trinco_piscina_criancas);
    // Tentativa de entrada do user na piscina

  char buffer[MAX_MESSAGE_BUFFER_SIZE];
    snprintf(buffer, MAX_MESSAGE_BUFFER_SIZE - 1, "%d", info->i);

  if(capacity_criancas + 1 >= MAX_CAPACITY_PISCINA_CRIANCAS){
    // Não há espaço para o user na piscina

    thread_send_message_to_socket(info->socket_monitor, FAIL_PIS_CRIANCAS, buffer);
    pthread_mutex_unlock(&trinco_piscina_criancas);
    return false;
  }

  capacity_criancas++;

  thread_send_message_to_socket(info->socket_monitor, ENPIS_CRIANCAS, buffer);
  
  pthread_mutex_unlock(&trinco_piscina_criancas);
  
  pthread_mutex_lock(&trinco_piscina_criancas);
  // User está dentro da piscina, correr uma chance de acidente

  if(should_have_accident()){
    // User teve um acidente
    capacity_criancas--;
    thread_send_message_to_socket(info->socket_monitor, ACCID, buffer);
    pthread_mutex_unlock(&trinco_piscina_criancas);

    return true;
  }

  pthread_mutex_unlock(&trinco_piscina_criancas);

  pthread_mutex_lock(&trinco_piscina_criancas);
  // User está a sair da piscina, mandar mensagem de saída
  thread_send_message_to_socket(info->socket_monitor, EXPIS_CRIANCAS, buffer);
  capacity_criancas--;
  pthread_mutex_unlock(&trinco_piscina_criancas);

  return false;
}
