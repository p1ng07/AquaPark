#ifndef EVENTS_H
#define EVENTS_H

#include <stdio.h>

/*
  Summary: Escreve um evento (string null-terminada) num ficheiro
  Parameter: file - File handler com permissao de escrita
  Parameter: ... - Lista de argumentos de formatação como se fosse um printf
  exemplo:
  write_event(file_handler, "O juliao tinha %d bananas \n", 5)
 */
#define write_event(file, ...)                                                 \
  do {                                                                         \
    char buffer[200];                                                          \
    snprintf(buffer, 201, __VA_ARGS__);                                        \
    printf("EVENT: ");                                                         \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
    fputs(buffer, file);                                                       \
  } while (0)

#endif
