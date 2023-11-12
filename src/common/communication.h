#ifndef COMMUNICATION_H
#define COMMUNICATION_H

typedef enum MType {
  EVENT,
  LOG
} MessageType;

// Macro que recebe um communication type e transforma numa string
#define string_from_com_type(type) #type

/*
  Summary: Recebe e interpreta mensagens do simulador.
  Esta interpretação passa por atualizar dados, escrever eventos, etc
*/
void poll_and_interpret_client_messages(int* fd_cliente);

int create_socket_and_wait_for_client_connection(int* server_socket, int* fd_cliente);

/* Lê nbytes de um ficheiro/socket.
   Bloqueia até conseguir ler os nbytes ou dar erro */
int readn(int fd, char *ptr, int nbytes);

#endif
