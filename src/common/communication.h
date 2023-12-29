#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdbool.h>
#include <stdint.h>

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

  ENWCD, // User entrou na fila de espera wc de deficientes, formato: "id,vip",
         // vip = 1 signifca que user que entrou é vip
  EXWCD, // User usou e saiu da wc de deficientes, formato: "id,vip"
  DESIS_WCD, // Um user desistiu da fila de espera da wc deficientes, formato:
             // "id"

  ENWCH, // User entrou na fila de espera da wc de homens, formato: "id,vip",
         // vip = 1 signifca que user que entrou é vip
  EXWCH, // User usou e saiusaiu da wc de homens, formato: "id,vip"
  DESIS_WCH, // Um user desistiu da fila de espera do togoba pequeno, formato:
             // "id"

  ENWCW, // User entrou na fila de espera da wc de mulheres, formato: "id,vip",
         // vip = 1 signifca que user que entrou é vip
  EXWCW, // User usou e saiu da wc de mulheres, formato: "id,vip"
  DESIS_WCW, // Um user desistiu da fila de espera da WC women, formato: "id"

  ENTBP, // User entrou na fila de espera do  tobogan pequeno, formato:
         // "id,vip", vip = 1 signifca que user que entrou é vip
  EXTBP,     // User usou e saiu no tobogan pequeno, formato: "id,vip"
  DESIS_TBP, // Um user desistiu da fila de espera do togoba pequeno, formato:
             // "id"

  ENBAR,    // User entrou no bar
  EXBAR,    // User usou e saiu do bar

  ENPIS,    // User entrou na piscina
  EXPIS,    // User usou e saiu da piscina
  FAIL_PIS, // User tentou entrar na piscina mas já estava no máximo

  ENPIS_CRIANCAS,    // User entrou na piscina das crianças
  EXPIS_CRIANCAS,    // User usou e saiu da piscina das crianças
  FAIL_PIS_CRIANCAS, // User tentou entrar na piscina das crianças mas já estava
		     // no máximo

  ENTBG, // User entrou na fila de espera do tobogan grande, formato: "id,vip"
         // = 1 signifca que user que entrou é vip
  EXTBG, // User usou e saiu do toboga grande, formato: "id,vip"
  DUTBG, // Ocorreu uma viagem de dois users no toboga grande
  INTBG, // Ocorreu uma viagem de um user no toboga grande
  DESIS_TBG, // Um user desistiu da fila de espera do togoba grande, formato:
             // "id"
} MessageType;

typedef struct {
  uint64_t entradas_parque;
  uint64_t saidas_parque;
  uint64_t desistencias;
  uint64_t acidentes;
  bool running_simulation; // Informa se a simulação está a decorrer ou não
} stats_info;

typedef struct {
  int *fd_cliente;
  stats_info *stats;
} communication_thread_args;

/*
  Summary: Recebe e interpreta mensagens do simulador.
  Esta interpretação passa por atualizar dados, escrever eventos, etc
*/
void poll_and_interpret_client_messages(communication_thread_args *args);

int create_socket_and_wait_for_client_connection(int *server_socket,
                                                 int *fd_cliente);

/* Lê nbytes de um ficheiro/socket.
   Bloqueia até conseguir ler os nbytes ou dar erro */
int readn(int fd, char *ptr, int nbytes);

/*
  Summary: Envia uma mensagem com um certo identificador para uma socket
  AVISO: O tamanho da mensagem deve ser <= MAX_MESSAGE_BUFFER_SIZE - 1
*/
void send_message_to_socket(int *socket, MessageType type, char *message);

#endif
