#include "communication.h"
#include "../monitor/events.h"
#include "common.h"
#include "string.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// Thread de comunicação que lê as mensagens do simulador/users para serem
// interpretadas no monitor
void poll_and_interpret_client_messages(communication_thread_args *args) {
  // Ler mensagem vinda do simulador
  char buffer[MAX_MESSAGE_BUFFER_SIZE];

#define IDENTIFIER_LENGTH 1

  // Abrir handlers para ficheiros de eventos, um por cada atração, e um para
  // eventos gerais
  FILE *file_eventos = fopen(args->file_eventos, "a");

  FILE *piscina_eventos = fopen(FICHEIRO_PISCINA_EVENTOS, "a");
  FILE *piscina_criancas_eventos =
      fopen(FICHEIRO_PISCINA_CRIANCAS_EVENTOS, "a");
  FILE *tobogan_grande_eventos = fopen(FICHEIRO_TOBOGAN_GRANDE_EVENTOS, "a");
  FILE *tobogan_pequeno_eventos = fopen(FICHEIRO_TOBOGAN_PEQUENO_EVENTOS, "a");
  FILE *wc_def_eventos = fopen(FICHEIRO_WC_DEF_EVENTOS, "a");
  FILE *wc_men_eventos = fopen(FICHEIRO_WC_MEN_EVENTOS, "a");
  FILE *wc_women_eventos = fopen(FICHEIRO_WC_WOMEN_EVENTOS, "a");
  FILE *bar_eventos = fopen(FICHEIRO_BAR_EVENTOS, "a");

  if (piscina_eventos == NULL || piscina_criancas_eventos == NULL ||
      tobogan_grande_eventos == NULL || tobogan_pequeno_eventos == NULL ||
      wc_def_eventos == NULL || wc_men_eventos == NULL ||
      wc_women_eventos == NULL || bar_eventos == NULL) {
    printf("Não foi possível abrir os ficheiros de eventos para escrita.");
    exit(1);
  }

    // TODO Adicionar um mecanismo para parar esta thread, provavelmente com
    // sinais
    // Ideia: Receber um SIGUSRX que muda a variavel de controlo do while para
    // false

    while (1) {
      // Ler mensagem com MAX_MESSAGE_BUFFER_SIZE de tamanho
      int n = readn(*args->fd_cliente, buffer, MAX_MESSAGE_BUFFER_SIZE);

      if (n > 0) {

        char message[MAX_MESSAGE_BUFFER_SIZE];
        strncpy(message, buffer + IDENTIFIER_LENGTH,
                MAX_MESSAGE_BUFFER_SIZE - IDENTIFIER_LENGTH);

        // O primeiro carater é o identificador da mensagem
        int identifier = -1;
        identifier = buffer[0];

        switch (identifier) {
	case EXITU: {
	  // Write a line to the event file
          char string[100];
          snprintf(string, 100, "EXIT: User %d exited the park.\n",
                   atoi(message));
          fputs(string, file_eventos);
          args->stats->saidas_parque++;
          break;
        }

        case ENDSM: {
	  args->stats->running_simulation = false;
          break;
	}

        case ENTER: {
          // Write a line to the event file
          char string[100];
          snprintf(string, 100, "ENTER: User %d entered the park.\n",
                   atoi(message));
          fputs(string, file_eventos);
          args->stats->entradas_parque++;
          break;
        }

        case ACCID: {
          // User had an accident
          char string[100];
          snprintf(string, 100, "Accident: User %d had an accident.\n",
                   atoi(message));
          fputs(string, file_eventos);
          args->stats->acidentes++;
          break;
        }

        case DESIS: {
          // User abandoned a waiting queue
          char string[100];
          snprintf(string, 100,
                   "Abandonment: User %d abandoned their waiting queue.\n",
                   atoi(message));
          fputs(string, file_eventos);
          args->stats->desistencias++;
          break;
        }

        case ENWCW: {
          // User entered the women bathroom
          char string[100];

          snprintf(string, 100, "WOMEN WC: User %d entered waiting queue.\n",
                   atoi(message));
          fputs(string, wc_women_eventos);
          break;
        }

        case EXWCW: {
          // User exited the women bathroom
          char string[100];
          snprintf(string, 100, "WOMEN WC: User %d used and exited.\n",
                   atoi(message));
          fputs(string, wc_women_eventos);
          break;
        }

        case ENWCH: {
          // User entered the men bathroom
          char string[100];

          snprintf(string, 100, "MEN WC: User %d entered waiting queue.\n",
                   atoi(message));
          fputs(string, wc_men_eventos);
          break;
        }

        case EXWCH: {
          // User exited the men bathroom
          char string[100];
          snprintf(string, 100, "MEN WC: User %d used and exited.\n",
                   atoi(message));
          fputs(string, wc_men_eventos);
          break;
        }

        case ENWCD: {
          // User entered the def bathroom
          char string[100];
          snprintf(string, 100, "Disabled WC: User %d entered waiting queue.\n",
                   atoi(message));
          fputs(string, wc_def_eventos);
          break;
        }

        case EXWCD: {
          // User exited the disabled bathroom
          char string[100];
          snprintf(string, 100, "Disabled WC: User %d used and exited.\n",
                   atoi(message));
          fputs(string, wc_def_eventos);
          break;
        }

        case ENTBP: {
          // User entered tobogan pequeno
          char string[100];
          int id = atoi(strtok(message, ","));
          bool vip = (bool)atoi(strtok(NULL, ","));

          if (vip) {
            snprintf(string, 100,
                     "Tobogan pequeno: VIP User %d entered waiting queue.\n",
                     id);
          } else {
            snprintf(string, 100,
                     "Tobogan pequeno: User %d entered waiting queue.\n", id);
          }

          fputs(string, tobogan_pequeno_eventos);
          break;
        }

        case EXTBP: {
          // User exited tobogan pequeno
          char string[100];
          int id = atoi(strtok(message, ","));
          bool vip = (bool)atoi(strtok(NULL, ","));

          if (vip) {
            snprintf(string, 100,
                     "Tobogan pequeno: VIP User %d used and exited.\n", id);
          } else {
            snprintf(string, 100, "Tobogan pequeno: User %d used and exited.\n",
                     id);
          }

          fputs(string, tobogan_pequeno_eventos);
          break;
        }
        case ENTBG: {
          /*User entrou na fila de espera do tobogan grande, formato : "id,vip",
           * vip = 1 signifca que user que entrou é vip
           */
          char string[100];
          int id = atoi(strtok(message, ","));
          bool vip = (bool)atoi(strtok(NULL, ","));

          if (vip) {
            snprintf(string, 100,
                     "Tobogan grande: VIP User %d entered waiting queue.\n",
                     id);
          } else {
            snprintf(string, 100,
                     "Tobogan grande: User %d entered waiting queue.\n", id);
          }

          fputs(string, tobogan_grande_eventos);
          break;
        }

        case EXTBG: {
          // User usou e saiu no tobogan pequeno, formato: "id,vip"
          char string[100];
          int id = atoi(strtok(message, ","));
          bool vip = (bool)atoi(strtok(NULL, ","));

          if (vip) {
            snprintf(string, 100,
                     "Tobogan grande: VIP User %d used and exited.\n", id);
          } else {
            snprintf(string, 100, "Tobogan grande: User %d used and exited.\n",
                     id);
          }

          fputs(string, tobogan_grande_eventos);
          break;
        }
        case DUTBG: {
          // Ocorreu uma viagem de dois users no toboga grande
          char string[100];
          snprintf(
              string, 100,
              "Toboga grande: Ocorreu uma viagem com dois utilizadores.\n");
          fputs(string, tobogan_grande_eventos);
          break;
        }
        case INTBG: {
          // Ocorreu uma viagem de um user no toboga grande
          char string[100];
          snprintf(string, 100,
                   "Toboga grande: Ocorreu uma viagem com um utilizador.\n");
          fputs(string, tobogan_grande_eventos);
          break;
        }
        case ERROR: {
          printf("\n------------------------------------\n");
          printf("\nOCORREU UM ERRO FATAL\n");
          printf("\nERRO: %s", message);
          printf("\n------------------------------------\n");

          args->stats->running_simulation = false;
        }
        default:
          fprintf(stderr, "Message type '%d' is not defined to be received.\n",
                  identifier);
          assert(0);
          break;
        }
      }
    };

    fclose(file_eventos );

    fclose(piscina_eventos);
    fclose(piscina_criancas_eventos);
    fclose(tobogan_grande_eventos);
    fclose(tobogan_pequeno_eventos);
    fclose(wc_def_eventos);
    fclose(wc_men_eventos);
    fclose(wc_women_eventos);
    fclose(bar_eventos);
}

/*
  Summary: Cria uma socket e espera por uma conexao do servidor
  Returns: Socket criada e file descriptor da socket do cliente atraves dos
  parametros
 */
int create_socket_and_wait_for_client_connection(int *server_socket,
                                                 int *fd_cliente) {
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
  if ((*fd_cliente = accept(*server_socket, (struct sockaddr *)&saun,
                            (socklen_t *)&len)) < 0) {
    perror("server: accept");
    return 1;
  }

  printf("Recebida conexão do simulador\n");

  return 0;
}

/* Lê nbytes de um ficheiro/socket.
   Bloqueia até conseguir ler os nbytes ou dar erro */
int readn(int fd, char *ptr, int nbytes) {
  int nleft, nread;

  nleft = nbytes;
  while (nleft > 0) {
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
void send_message_to_socket(int *socket, MessageType type, char *message) {
  char buffer[MAX_MESSAGE_BUFFER_SIZE] = {type};
  send(*socket, strncat(buffer, message, MAX_MESSAGE_BUFFER_SIZE - 1),
       MAX_MESSAGE_BUFFER_SIZE, 0);
}
