#include "../common/configuration.h"
#include "../common/communication.h"
#include "../common/common.h"
#include "../common/string.h"
#include "print.h"
#include "user.h"
#include <assert.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>

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
    // Ler código identificador do tipo de mensagem (primeiras 5 letras da
    // mensagem)

    if (strncmp(buffer, "BEGIN", 5) == 0) {
      break;
    }else if (strncmp(buffer, "ENDSM", 5) == 0){
      printf("Ordem de monitor: Terminar simulação.");
      scanf("");
      exit(0);
      break;
    }
  }
}

int main(int argc, char* argv[]) {

  if (argc < 2) {
    printf("ERRO: Não foi especificado um ficheiro de configuração");
    return 1;
  }

  // Carregar parametros
  configuration conf = extract_config_from_file(argv[1]);
  conf_parameter* num_users_inicial = get_parameter_from_configuration(&conf, new_str("users_inicias"));

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

  pthread_t user_thread_list[MAX_THREADS];

  int* allocated_client_socket = malloc(sizeof(int));
  *allocated_client_socket = client_socket;


  // Criacao de threads
  for (int i = 0; i < num_users_inicial->i; i++) {
    user_entry_point_info *info_send = malloc(sizeof(user_entry_point_info));
    info_send->socket_monitor = allocated_client_socket;
    info_send->i = i;

    pthread_create(&user_thread_list[i], NULL,(void*)user_entry_point,info_send);
  }

  // Esperar que threads acabem
  for (int i = 0; i < num_users_inicial->i; i++) {
    pthread_join(user_thread_list[i], NULL);
  }

  send_message_to_socket(&client_socket, ERROR, "Simulador fechou.");

  return 0;
}
