#include <stdbool.h>
#include "../common/configuration.h"
#include "../common/communication.h"
#include "../common/common.h"
#include "../common/string.h"
#include "bathrooms.h"
#include "print.h"
#include "toboga_grande.h"
#include "toboga_pequeno.h"
#include "user.h"
#include <assert.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>

/*
  Summary: Espera pela mensagem de começo vinda do monitor, pode também receber
  uma mensagem para fechar a simulação
*/
void wait_for_begin_message(int socket) {
  char buffer[MAX_MESSAGE_BUFFER_SIZE];

  // TODO Adicionar um mecanismo para parar esta thread, provavelmente com
  // sinais

  while (1) {
    // Ler mensagem com MAX_MESSAGE_BUFFER_SIZE de tamanho
    int n = readn(socket, buffer, MAX_MESSAGE_BUFFER_SIZE);
    // Ler código identificador do tipo de mensagem (primeiro carater da mensagem)
    int identifier = buffer[0];

    if (identifier == BEGIN) {
      break;
    }else if (identifier == ENDSM){
      printf("Ordem de monitor: Terminar simulação.");
      exit(0);
      break;
    }
  };
}

bool parque_aberto = true;

  // Contador que mantém o número atual de utilizadores no parque
int global_user_counter = 0;

unsigned long *global_user_thread_list = NULL;
pthread_mutex_t global_user_thread_list_mutex = PTHREAD_MUTEX_INITIALIZER;

// Define a % chance (0 a 100) de haver um acidente
int have_accident_parameter = -1;

// Define a % chance (0 a 100) de um user desistir de uma fila de espera
int quit_attraction_parameter = -1;

int main(int argc, char *argv[]) {

  if (argc < 2) {
    printf("ERRO: Não foi especificado um ficheiro de configuração");
    return 1;
  }

    // Alloca espaço para informação de threads
  global_user_thread_list = calloc(MAX_THREADS,sizeof(pthread_t));

  // Carregar parametros
  configuration conf = extract_config_from_file(argv[1]);
  conf_parameter *num_users_inicial =
      get_parameter_from_configuration(&conf, new_str("users"));

  assert(num_users_inicial != NULL);

  conf_parameter *accident_parameter =
      get_parameter_from_configuration(&conf, new_str("accident"));

  assert(accident_parameter != NULL);
  have_accident_parameter = accident_parameter->i;

  conf_parameter *quit_parameter =
      get_parameter_from_configuration(&conf, new_str("quit"));

  assert(quit_parameter != NULL);
  quit_attraction_parameter = quit_parameter->i;

  assert(num_users_inicial->i <= MAX_THREADS);

  int client_socket, len;
  struct sockaddr_un saun;

  if ((client_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    perror("client: socket");
    return 1;
  }

  saun.sun_family = AF_UNIX;
  strcpy(saun.sun_path, ADDRESS_SOCKET);

  len = sizeof(saun.sun_family) + strlen(saun.sun_path);

  if (connect(client_socket, (const struct sockaddr*)&saun, len) < 0) {
    perror("client: connect");
    return 1;
  }

  // Esperar por uma mensagem de começo
  wait_for_begin_message(client_socket);

  pthread_t worker_threads[NUMBER_OF_WORKER_THREADS];

  // Criar worker threads para as atrações que precisam delas
  pthread_create(&worker_threads[DEF_WC_THREAD], NULL,
		 (void*)disabled_bathroom_worker_entry_point, NULL);

  // Workers wc homens, existe um worker por casa de banho
  pthread_create(&worker_threads[MEN_WC_THREAD_1], NULL,
		 (void*)men_bathroom_worker_entry_point, NULL);
  pthread_create(&worker_threads[MEN_WC_THREAD_2], NULL,
		 (void*)men_bathroom_worker_entry_point, NULL);

  // Workers WC mulheres, existe um worker por casa de banho
  pthread_create(&worker_threads[WOMEN_WC_THREAD_1], NULL,
		 (void*)women_bathroom_worker_entry_point, NULL);
  pthread_create(&worker_threads[WOMEN_WC_THREAD_2], NULL,
		 (void*)women_bathroom_worker_entry_point, NULL);

  // Worker tobogan pequeno
  pthread_create(&worker_threads[TOBOGAN_PEQUENO_THREAD], NULL,
		 (void*)tobogan_pequeno_worker_entry_point, NULL);
  
  int* allocated_client_socket = malloc(sizeof(int));
  *allocated_client_socket = client_socket;

  // Worker tobogan grande
  pthread_create(&worker_threads[TOBOGAN_GRANDE_THREAD], NULL,
		 (void*)tobogan_grande_worker_entry_point, allocated_client_socket);
  

  // TODO Adicionar hora local de inicio de simulação, horas para cada diversão
  // estar aberta, e mudar "parque_aberto" consoante as horas, qualquer o
  // intervalo de cada hora com base num parametro de hora_inicial hora_final e
  // tempo_de_simulação

  // Contador que se usa para criar um id única para cada utilizador
  int counter_id_user = 0;

  // Criacao de threads
  for (int i = 0; i < num_users_inicial->i; i++) {
    user_info *info_send = malloc(sizeof(user_info));
    info_send->socket_monitor = allocated_client_socket;
    info_send->i = counter_id_user++;

    info_send->deficient = rand() < (float)RAND_MAX * 0.19f;
    info_send->is_man = rand() < (float)RAND_MAX * 0.50;

    // Idade pode ir dos 10 aos 80
    info_send->age = (rand() % 80) + 10;

    pthread_create(&global_user_thread_list[i], NULL,(void*)user_entry_point,info_send);
    info_send->pthread_info = global_user_thread_list[i];
  }

  time_t atual = 0;
  time_t inicio = 0;
  time(&inicio);

  conf_parameter* tempo_simulacao = get_parameter_from_configuration(&conf, new_str("tempo_simulacao"));

  do {
    time(&atual);

    pthread_mutex_lock(&global_user_thread_list_mutex);
    for (int i = 0; i < MAX_THREADS; i++) {
      // Chance de 50% de tentar criar um user novo no parque
      if(rand() % 2 == 0)
	break;

      if(global_user_thread_list[i] == 0){
	user_info *info_send = malloc(sizeof(user_info));
	info_send->socket_monitor = allocated_client_socket;
        info_send->i = counter_id_user++;

        info_send->deficient = rand() < (float)RAND_MAX * 0.19f;
        info_send->is_man = rand() < (float)RAND_MAX * 0.50;

        // Idade pode ir dos 10 aos 80
        info_send->age = (rand() % 80) + 10;

        pthread_create(&global_user_thread_list[i], NULL,
		       (void *)user_entry_point, info_send);
	info_send->pthread_info = global_user_thread_list[i];
	break;
      }
    }
    pthread_mutex_unlock(&global_user_thread_list_mutex);

  } while (difftime(atual, inicio) < tempo_simulacao->i);

  parque_aberto = false;


  // Forçar que as threads acabem
  for (int i = 0; i < MAX_THREADS; i++) {
    if (global_user_thread_list[i] != 0){
	pthread_cancel(global_user_thread_list[i]);
    }
  }

  send_message_to_socket(&client_socket, ENDSM, "");

  return 0;
}
