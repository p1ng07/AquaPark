#include "common.h"
#include "communication.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>

void poll_and_interpret_client_messages(int* fd_cliente) {
  printf("Comecada thread para ler messagens: %i", *fd_cliente);
  // Ler mensagem vindo do simulador
  char buffer[MAX_MESSAGE_BUFFER_SIZE];

  while (1) {

    // TOOD usar o readn para aceitar messagens do simulador
    int n = readn(*fd_cliente, buffer, MAX_MESSAGE_BUFFER_SIZE);

    // TODO Ler mensagemm, escrever e depois bloquear o processo
    if (n > 0){
	printf("Messagem do simulador: %s\n", buffer);
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
  int fromlen;
  if ((*fd_cliente = accept(*server_socket, (struct sockaddr *) &saun, (socklen_t*)&fromlen)) < 0) {
    perror("server: accept");
    return 1;
  }
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
