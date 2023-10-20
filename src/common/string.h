#ifndef STRING_H
#define STRING_H

// String baseada em tamanho
typedef struct {
  char *value;
  int length;
} str;

/* Summary: Escreve uma str terminada em '\n' para a consola 
 */
void str_print(str string);
#endif
