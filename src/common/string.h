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

/* Summary: Compara duas str
   Retorna: 1 se ambas são iguais, 0 se são diferentes
 */
int str_equals(str* str1, str* str2);

/*
  AVISO: O apontador retornado terá de ser manualmente libertado com str_free()
 */
str* new_str(char* null_term_string);

void str_free(str* str);
#endif
