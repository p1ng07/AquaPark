#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdio.h>
typedef enum MType {
  EVENT,
  MESNG,
  ERROR,
  ENTER, // Utilizador entrou no parque, formato: "ENTER id"
  BEGIN, // Começar simulação
  ENDSM  // Acabar simulação
} MessageType;

typedef struct {
  int* fd_cliente;
  char* file_eventos;
} communication_thread_args;

// Macro que recebe um communication type e transforma numa string
#define string_from_com_type(TYPE) #TYPE
#define xstring_from_com_type(TYPE) string_from_com_type(TYPE)

/*
  Summary: Recebe e interpreta mensagens do simulador.
  Esta interpretação passa por atualizar dados, escrever eventos, etc
*/
void poll_and_interpret_client_messages(communication_thread_args *args);

int create_socket_and_wait_for_client_connection(int* server_socket, int* fd_cliente);

/* Lê nbytes de um ficheiro/socket.
   Bloqueia até conseguir ler os nbytes ou dar erro */
int readn(int fd, char *ptr, int nbytes);

/*
  Summary: Envia uma mensagem com um certo identificador para uma socket
  AVISO: O tamanho da mensagem deve ser <= MAX_MESSAGE_BUFFER_SIZE - 5
*/
void send_message_to_socket(int*socket, MessageType type, char* message);

#endif
