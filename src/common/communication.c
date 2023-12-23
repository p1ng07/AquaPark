#include "common.h"
#include "communication.h"
#include "../monitor/events.h"
#include "string.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>

// Thread de comunicação que lê as mensagens do simulador/users para serem interpretadas no monitor
void poll_and_interpret_client_messages(communication_thread_args* args) {
  printf("Comecada thread para ler messagens.\n");
  // Ler mensagem vindo do simulador
  char buffer[MAX_MESSAGE_BUFFER_SIZE];

  args->stats->entradas = 69;

  FILE* file_eventos = fopen(args->file_eventos, "a");

  // TODO Adicionar um mecanismo para parar esta thread, provavelmente com
  // sinais
  // Ideia: Receber um SIGUSRX que muda a variavel de controlo do while para
  // false

  while (1) {

    // Ler mensagem com MAX_MESSAGE_BUFFER_SIZE de tamanho
    int n = readn(*args->fd_cliente, buffer, MAX_MESSAGE_BUFFER_SIZE);

    if (n > 0) {

      // Os primeiros 5 carateres sao o identificador da mensagem
      char message[MAX_MESSAGE_BUFFER_SIZE];
      strncpy(message, buffer + 5, MAX_MESSAGE_BUFFER_SIZE - 5);
      char identifier[5];
      strncpy(identifier, buffer, 5);

      if (strncmp(buffer, "EVENT", 5) == 0) {
        // Evento, TODO escrever no ficheiro
        printf("EVENTO: %s \n", buffer + 5);
      } else if (strncmp(buffer, "MESNG", 5) == 0) {
        printf("LOG: %s \n", buffer + 5);
      } else if (strncmp(buffer, "ERROR", 5) == 0) {
        printf("ERRO: %s \n", buffer + 5);
      } else if (strncmp(buffer, "EXITU", 5) == 0) {
	// Escrever uma linha para o ficheiro de eventos
	char string[100];
	snprintf(string, 100, "EXIT: User %d saiu do parque.\n", atoi(message));
	printf("%s", string);
	fputs(string, file_eventos);

      } else if (strncmp(buffer, "ENTER", 5) == 0) {
	// Escrever uma linha para o ficheiro de eventos
	char string[100];
	snprintf(string, 100, "ENTER: User %d entrou no parque.\n", atoi(message));
	printf("%s", string);
	fputs(string, file_eventos);

      }else{
	fprintf(stderr, "Tipo de mensagem '%s' não está declarado para receber\n",
		identifier);
	assert(0);
      }
    }
  }
}

/*
  Summary: Cria uma socket e espera por uma conexao do servidor
  Returns: Socket criada e file descriptor da socket do cliente atraves dos parametros
 */
int create_socket_and_wait_for_client_connection(int* server_socket, int* fd_cliente){
  int len;
  // Socket
  struct sockaddr_un saun;

  // Criar socket unix
  if ((*server_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    perror("server: socket");
    return 1;
  }

  // Permitir a reutilização da socket (redundante porque já fazemos o unlink()
  // à frente)
  int options = 1;
  setsockopt(*server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&options,
             sizeof(options));

  saun.sun_family = AF_UNIX;
  strcpy(saun.sun_path, ADDRESS_SOCKET);

  // "Unlink" conexoes que já existissem a sockets com este address
  unlink(ADDRESS_SOCKET);
  len = sizeof(saun.sun_family) + strlen(saun.sun_path);

  // Dar um nome (saun) à socket
  if (bind(*server_socket, (struct sockaddr *)&saun, len) < 0) {
    perror("server: bind");
    return 1;
  }

  // Mete o servidor num estado passivo há espera de conexoes
  if (listen(*server_socket, 5) < 0) {
    perror("server: listen");
    return 1;
  }

  // Bloqueia até receber uma conexao de um cliente
  if ((*fd_cliente = accept(*server_socket, (struct sockaddr *) &saun, (socklen_t*)&len)) < 0) {
    perror("server: accept");
    return 1;
  }

  printf("Recebida conexão do simulador\n");
  
  return 0;
}

/* Lê nbytes de um ficheiro/socket.
   Bloqueia até conseguir ler os nbytes ou dar erro */
int readn(int fd, char *ptr, int nbytes)
{
	int nleft, nread;

	nleft = nbytes;
	while (nleft > 0)
	{
		nread = read(fd, ptr, nleft);
		if (nread < 0)
			return (nread);
		else if (nread == 0)
			break;

		nleft -= nread;
		ptr += nread;
	}
	return (nbytes - nleft);
}

// Envia uma mensagem para uma dada socket
void send_message_to_socket(int*socket, MessageType type, char* message) {
  if(type == EVENT){
    char buffer[MAX_MESSAGE_BUFFER_SIZE] = "EVENT";
    send(*socket,
	 strncat(buffer, message, MAX_MESSAGE_BUFFER_SIZE - 1),
         MAX_MESSAGE_BUFFER_SIZE, 0);
  }else if(type == MESNG){
    char buffer[MAX_MESSAGE_BUFFER_SIZE] = "MESNG";
    send(*socket,
	 strncat(buffer, message, MAX_MESSAGE_BUFFER_SIZE - 1),
         MAX_MESSAGE_BUFFER_SIZE, 0);
  }else if (type == ERROR){
    char buffer[MAX_MESSAGE_BUFFER_SIZE] = "ERROR";
    send(*socket,
	 strncat(buffer, message, MAX_MESSAGE_BUFFER_SIZE - 1),
         MAX_MESSAGE_BUFFER_SIZE, 0);
  }else if (type == BEGIN){
    char buffer[MAX_MESSAGE_BUFFER_SIZE] = "BEGIN";
    send(*socket,
	 strncat(buffer, message, MAX_MESSAGE_BUFFER_SIZE - 1),
         MAX_MESSAGE_BUFFER_SIZE, 0);
  }else if (type == ENTER){
    char buffer[MAX_MESSAGE_BUFFER_SIZE] = "ENTER";
    send(*socket,
	 strncat(buffer, message, MAX_MESSAGE_BUFFER_SIZE - 1),
         MAX_MESSAGE_BUFFER_SIZE, 0);
  }else if (type == EXITU){
    char buffer[MAX_MESSAGE_BUFFER_SIZE] = "EXITU";
    send(*socket,
	 strncat(buffer, message, MAX_MESSAGE_BUFFER_SIZE - 1),
         MAX_MESSAGE_BUFFER_SIZE, 0);
  }else if (type == ENDSM){
    char buffer[MAX_MESSAGE_BUFFER_SIZE] = "ENDSM";
    send(*socket,
	 strncat(buffer, message, MAX_MESSAGE_BUFFER_SIZE - 1),
         MAX_MESSAGE_BUFFER_SIZE, 0);
  }else{
    fprintf(stderr,"Tipo de mensagem '%i' não está declarado para envio\n", type);
    assert(0);
  }
}
