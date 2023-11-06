#include "../common/common.h"
#include "../common/configuration.h"
#include "events.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

// TODO
/*
  Summary: Recebe e interpreta mensagens do simulador.
  Esta interpretação passa por atualizar dados, escrever eventos, etc
*/
void poll_and_interpret_client_messages(int fd_cliente);

int main(int argc, char *argv[]) {

  if (argc < 3) {
    perror("ERRO: Têm de ser especificados ficheiros de configuração e de "
           "escrita de eventos, nessa ordem");
    return 1;
  }

  // Carregar configuração de ficheiro
  configuration _conf = extract_config_from_file(argv[1]);

  int server_socket, len;
  // Socket
  struct sockaddr_un saun, fsaun;

  // Criar socket unix
  if ((server_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    perror("server: socket");
    return 1;
  }

  // Permitir a reutilização da socket (redundante porque já fazemos o unlink()
  // à frente)
  int options = 1;
  setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&options,
             sizeof(options));

  saun.sun_family = AF_UNIX;
  strcpy(saun.sun_path, ADDRESS_SOCKET);

  // "Unlink" conexoes que já existissem a sockets com este address
  unlink(ADDRESS_SOCKET);
  len = sizeof(saun.sun_family) + strlen(saun.sun_path);

  // Dar um nome (saun) à socket
  if (bind(server_socket, &saun, len) < 0) {
    perror("server: bind");
    return 1;
  }

  // Mete o servidor num estado passivo há espera de conexoes
  if (listen(server_socket, 5) < 0) {
    perror("server: listen");
    return 1;
  }
  // Bloqueia até receber uma conexao de um cliente
  int fd_cliente, fromlen;
  if ((fd_cliente = accept(server_socket, &fsaun, &fromlen)) < 0) {
    perror("server: accept");
    return 1;
  }

  // Lançar um process filho para lidar com messagens do simulador

  // TODO o monitor deve ter uma representação do simulador (atracoes, numero de pessoas, etc.)
  // Esta representação deve ser atualizada pelo processo filho que lida com messagens do cliente
  // Sempre que o processo filho recebe uma atualização dos dados, deve mandar um sinal aoa pai (SIGUSR) para fazer refresh do ecra
  // O processo filho deve também lidar com eventos (escrever os eventos para o ficheiro de eventos)
  int childpid = -1;
  if ((childpid = fork()) < 0) {
    perror("Não foi possível criar o processo que lida com as messagens do "
	   "simulador.");
    exit(1);
  } else if (childpid == 0) {
    // Processo foi criado
    // Marcar o child process para morrer quando o pai morrer
    prctl(PR_SET_PDEATHSIG, SIGHUP);
    poll_and_interpret_client_messages(fd_cliente);
  }

  char buffer[MAX_MESSAGE_BUFFER_SIZE] = "Messagem";


  // TODO Em vez de fazer isto, criar um processo à parte que apenas imprime mensagens vindas do cliente
  // TODO Alternativa

  printf("%s", buffer);

  // File handler com opção "append" (leitura, escrita, juntar ao fim do
  // ficheiro)
  FILE *file_eventos = fopen(argv[2], "a");

  bool menu_principal_running = true;

  while (menu_principal_running) {
    printf("1 - Iniciar simulação\n");
    printf("2 - Terminar simulação\n");
    // TODO: Adicionar opcoes de passar horas quando a simulação está a
    // correr
    printf("3 - Reiniciar ficheiro de eventos\n");
    printf("4 - Passar hora na simulação\n");
    printf("5 - Passar x horas na simulação\n");
    printf("Opcao: ");

    int input_choice = get_menu_input(0, 4);

    switch (input_choice) {
    case 1:
      // TODO Iniciar simulação
      break;
    case 2:
      // TODO Terminar simulaçã̀o
      break;
    case 3:
      // TODO Limpar ficheiro de eventos

      // Reabrir o ficheiro em modo de escrita faz com que os conteúdos sejam
      // apagados
      freopen(argv[2], "w", file_eventos);
      freopen(argv[2], "a", file_eventos);
      break;
    case 4:
      // TODO Mandar sinal para passar uma hora na simulação
      break;
    case 5:
      // TODO Mandar sinal para passar x horas na simulação
      printf("\n Quantas horas devem passar? ");
      int elapsed_hours = get_menu_input(0, INT_MAX);
      break;
    }
  }

  write_event(file_eventos, "O Julio é o Cbum, %s \n", "e o miguel é o ramon");

  close(server_socket);
  return 0;
}

void poll_and_interpret_client_messages(int fd_cliente) {
  while(1){
    
    // Ler mensagem vindo do simulador
    char buffer[MAX_MESSAGE_BUFFER_SIZE];
    read(fd_cliente, buffer, sizeof(buffer));

    // TODO Interpretar mensagem
    printf("%s", buffer);
  }
}
