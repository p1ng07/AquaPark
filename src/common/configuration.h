#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "string.h"
#include <bits/types/FILE.h>

/* Representacao de cada parametro na configuração
   Cada parametro pode tomar o valor de um float, string ou numero inteiro
*/
typedef struct {
  str key;

  union {
    int i;
    str str;
  };
} conf_parameter;

/* Representacao da configuracao
 */
typedef struct {
  conf_parameter** list_of_parameters;
  int num_of_parameters;
} configuration;

/*
    Summary: Extrai uma configuração de um ficheiro dado
    Returns: configuracao em caso de sucesso, NULL em caso de erro
    ATENCAO: A lista dentro do struct e o apontador retornado precisam de ser
    libertados manualmente
*/
configuration extract_config_from_file(char* file_path);

/*
  Summary: Retorna o parametro da configuracao com o nome dado
  Returns: Apontador para o parametro pedido
  ERRO: Retorna NULL caso o parametro nao exista na configuração, ou em caso de erro
*/
conf_parameter *get_parameter_from_configuration(configuration *conf,
						 str *parameter_name);

/* Summary: Extrai um parametro de configuração a partir de uma linha terminada em '\n' 
   Retorna: Parametro em caso de sucesso, NULL em caso de falha
 */
conf_parameter *read_parameter_from_line(char *line);

int get_num_of_configuration_parameters_in_file(FILE *iter);

void free_configuration(configuration* conf);
#endif
