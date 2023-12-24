#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdio.h>
#include <stdbool.h>

// O "formato" de cada mensagem especifica que informação é enviada depois do
// identificador
typedef enum MType {
  EVENT,
  MESNG,
  ERROR,
  ENTER, // Utilizador entrou no parque, formato: "id"
  BEGIN, // Começar simulação
  ENDSM, // Acabar simulação
  EXITU, // Utilizador saiu do parque, formato: "id"

  DESIS, // User desistiu da sua fila de espera

  ENWCD, // User entrou na wc de deficientes, formato: "id"
  EXWCD, // User saiu da wc de deficientes, formato: "id"
} MessageType;

typedef struct {
  int entradas_parque;
  int saidas_parque;
  int desistencias;
  int acidentes;
  bool running_simulation; // Informa se a simulação está a decorrer ou não
}stats_info;

typedef struct {
  int* fd_cliente;
  char* file_eventos;
  stats_info* stats;
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
