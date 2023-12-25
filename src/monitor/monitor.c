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

  //-Estatísticas a partilhar com a thread de comunicação------------------
  communication_thread_args *args = malloc(sizeof(communication_thread_args));

  stats_info *stats = malloc(sizeof *stats);
  *stats = (stats_info){.acidentes = 0,
			.desistencias = 0,
			.entradas_parque = 0,
			.saidas_parque = 0,
			.running_simulation = false};

  args->stats = stats;
  args->fd_cliente = &fd_cliente;
  args->file_eventos = argv[2];

  int* allocated_fd_cliente = malloc(sizeof(int));
  *allocated_fd_cliente = fd_cliente;
  pthread_create(&reading_thread,0, (void*)poll_and_interpret_client_messages, args);
  //----------------------------------------------------------------------

  bool menu_principal_running = true;

  while (menu_principal_running) {
    printf("1 - Iniciar simulação\n");
    printf("2 - Limpar ficheiro de eventos\n");
    printf("3 - Terminar simulação\n");
    printf("Opcao: ");

    int input_choice = get_menu_input(0, 4);

    switch (input_choice) {
    case 1:
      // Iniciar simulação
      {
	char buffer[10];
	send_message_to_socket(&fd_cliente, BEGIN, buffer);
	menu_principal_running = false;
	args->stats->running_simulation = true;
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
	menu_principal_running = false;
      }
      break;
    }
  }

  // Mostrar stats enquanto a simulação está a correr

  int pontinhos = 0;

  while (args->stats->running_simulation) {
    // Fazer o refresh a cada 0.5 segundos
    printf("\033c");
    printf("Desistências: %ld\n", args->stats->desistencias);
    printf("Utilizadores no parque: %ld\n", args->stats->entradas_parque - args->stats->saidas_parque);
    printf("Acidentes ocorridos: %ld\n", args->stats->acidentes);
    printf("Entradas totais: %ld\n", args->stats->entradas_parque);
    printf("Saidas ocorridos: %ld\n", args->stats->saidas_parque);
    printf("Estado: A Correr\n");
    printf("------------------------------\n");

    // Cena estúpida
    for (int i = 0; i <= pontinhos; i++) {
      printf(".");
    }
    pontinhos++;
    pontinhos %= 3;

    fflush(stdout);
    usleep(5000);
  }

  // TOOD RETIRAR ISTO
  /* printf("\033c"); */
  printf("Entradas totais: %ld\n", args->stats->entradas_parque);
  printf("Desistências: %ld\n", args->stats->desistencias);
  printf("Acidentes ocorridos: %ld\n", args->stats->acidentes);
  printf("Estado: Acabada\n");

  close(server_socket);
  fclose(file_eventos);
  return 0;
}

