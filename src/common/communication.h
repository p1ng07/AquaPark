#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>
#include <stdbool.h>

// O "formato" de cada mensagem especifica que informação é enviada no buffer
// depois do identificador
typedef enum MType {
  ERROR, // Ocorreu um erro fatal, fechar monitor e simulação
  ENTER, // Utilizador entrou no parque, formato: "id"
  BEGIN, // Começar simulação
  ENDSM, // Acabar simulação
  EXITU, // Utilizador saiu do parque, formato: "id"

  ACCID, // Ocorreu um acidente

  DESIS, // User desistiu da sua fila de espera

  ENWCD, // User entrou na wc de deficientes, formato: "id,vip", vip = 1
         // signifca que user que entrou é vip
  EXWCD, // User saiu da wc de deficientes, formato: "id"

  ENWCH, // User entrou na wc de homens, formato: "id,vip", vip = 1
         // signifca que user que entrou é vip
  EXWCH, // User saiu da wc de homens, formato: "id"

  ENWCW, // User entrou na wc de mulheres, formato: "id,vip", vip = 1
         // signifca que user que entrou é vip
  EXWCW, // User saiu da wc de mulheres, formato: "id"

  ENTBP, // User entrou no tobogan pequeno, formato: "id", vip = 1
         // signifca que user que entrou é vip
  EXTBP, // User saiu no tobogan pequeno, formato: "id"
} MessageType;

typedef struct {
  uint64_t entradas_parque;
  uint64_t saidas_parque;
  uint64_t desistencias;
  uint64_t acidentes;
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
  AVISO: O tamanho da mensagem deve ser <= MAX_MESSAGE_BUFFER_SIZE - 1
*/
void send_message_to_socket(int*socket, MessageType type, char* message);

#endif
