#include "../common/configuration.h"
#include "../common/common.h"
#include "../common/string.h"
#include "print.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {

  if (argc < 2) {
    printf("ERRO: Não foi especificado um ficheiro de configuração");
    return 1;
  }

  int i, client_socket, len;
  struct sockaddr_un saun;

  if ((client_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    perror("client: socket");
    return 1;
  }

  saun.sun_family = AF_UNIX;
  strcpy(saun.sun_path, ADDRESS_SOCKET);

  len = sizeof(saun.sun_family) + strlen(saun.sun_path);

  if (connect(client_socket, &saun, len) < 0) {
    perror("client: connect");
    return 1;
  }

  configuration conf = extract_config_from_file(argv[1]);
  conf_parameter *param = get_parameter_from_configuration(&conf, new_str("nome"));

  char message[MAX_MESSAGE_BUFFER_SIZE] = "Isto é o simulador\n";
  
  scanf("%s",message);

  send(client_socket, message, strlen(message), 0);

  return 0;
}
