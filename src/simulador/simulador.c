#include "../common/configuration.h"
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

int main(int argc, char* argv[]) {

  if (argc < 2) {
    printf("ERRO: Não foi especificado um ficheiro de configuração");
    return 1;
  }

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

  configuration conf = extract_config_from_file(argv[1]);
  conf_parameter* num_users_inicial = get_parameter_from_configuration(&conf, new_str("users_inicias"));

  assert(num_users_inicial->i <= MAX_THREADS);

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

  char message[MAX_MESSAGE_BUFFER_SIZE];
  
  printf("Escreva uma mensagem para enviar: ");
  scanf("%s",message);
  printf("%s", message);

  send_string_to_monitor(&client_socket, EVENT, message);

  // Esperar que threads acabem
  for (int i = 0; i < num_users_inicial->i; i++) {
    pthread_join(user_thread_list[i], NULL);
  }

  send_string_to_monitor(&client_socket, ERROR, "Simulador fechou.");

  return 0;
}
