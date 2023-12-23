#include "../common/common.h"
#include "../common/configuration.h"
#include "../common/communication.h"
#include "./menu.h"
#include "events.h"
#include <event2/event.h>
#include <pthread.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

  if (argc < 3) {
    perror("ERRO: Têm de ser especificados ficheiros de configuração e de "
           "escrita de eventos, nessa ordem");
    return 1;
  }

  // Carregar configuração de ficheiro
  configuration conf = extract_config_from_file(argv[1]);

  // File handler com opção "append" (leitura, escrita, juntar ao fim do
  // ficheiro)
  FILE *file_eventos = fopen(argv[2], "a");

  int fd_cliente, server_socket = -1;
  printf("Esperando conexão do simulador.\n");
  create_socket_and_wait_for_client_connection(&server_socket, &fd_cliente);

  // Lançar uma thread para lidar com messagens do simulador
  pthread_t reading_thread;

  communication_thread_args* args = malloc(sizeof(communication_thread_args));
  args->stats = malloc(sizeof(stats_info));
  args->fd_cliente = &fd_cliente;
  args->file_eventos = argv[2];

  int* allocated_fd_cliente = malloc(sizeof(int));
  *allocated_fd_cliente = fd_cliente;
  pthread_create(&reading_thread,0, (void*)poll_and_interpret_client_messages, args);

  bool menu_principal_running = true;

  while (menu_principal_running) {
    printf("1 - Iniciar simulação\n");
    printf("2 - Limpar ficheiro de eventos\n");
    printf("3 - Terminar simulação\n");
    // TODO: Adicionar opcoes de passar horas quando a simulação está a
    // correr
    printf("Opcao: ");

    int input_choice = get_menu_input(0, 4);

    switch (input_choice) {
    case 1:
      // Iniciar simulação
      {
	char buffer[10];
	send_message_to_socket(&fd_cliente, BEGIN, buffer);
      }
      break;
    case 2:
      // Reabrir o ficheiro em modo de escrita faz com que os conteúdos sejam
      // apagados
      freopen(argv[2], "w", file_eventos);

      // Reabri-lo em modo append para poder ser escrito
      freopen(argv[2], "a", file_eventos);
      break;
    case 3:
      // Fechar simulação
      {
	char buffer[10];
	close(server_socket);
	return 0;
      }
      break;
    }
  }


  close(server_socket);
  fclose(file_eventos);
  return 0;
}

