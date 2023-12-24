#include "configuration.h"
#include "string.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Variavel global para tratamento de dados
extern int errno;

configuration extract_config_from_file(char* file_path){
  FILE* fptr = fopen(file_path, "r");

  // Se ficheiro não existir, programa não pode continuar
  if (fptr == NULL){
    printf("ERRO: Ficheiro de configuração %s não existe.", file_path);
    exit(1);
  }

  configuration conf = {};

  // Inicializar configuração
  int num_params = get_num_of_configuration_parameters_in_file(fptr);
  conf.list_of_parameters = malloc(sizeof(conf_parameter*) * num_params);
  conf.num_of_parameters = num_params;

  fseek(fptr, 0, SEEK_SET);

  // Tamanho maximo do buffer para ler a linha
#define max_length 150

  char line[max_length];

  // Ler todos os parametros do ficheiro
  int i = 0;
  while (fgets(line, max_length, fptr) != NULL){

    // Se usarmos line diretamente, esta char* alocada na stack sai de scope
    // antes de ser utilizada na configuração, então as strs que estariam a
    // apontar para ela, seriam inválidas
    char *heap_allocated_line = malloc(sizeof(char) * strlen(line));
    strcpy(heap_allocated_line, line);

    conf_parameter* param = read_parameter_from_line(heap_allocated_line);

    if (param != NULL){
	conf.list_of_parameters[i] = param;
	i++;
    }else{
      free(heap_allocated_line);
    }
  }

  fclose(fptr);
  return conf;
}

// Retorna um conf_parameter pronto a usar, ou NULL em caso de erro
// recebe uma string terminada em \n
conf_parameter *read_parameter_from_line(char *line) {
  int len_param_name = 0;

  for(int i = 0; i < strlen(line); i++){
    char curr_char = line[i];
    if (curr_char == '\n') {
	return NULL;
	break;
    } else if (curr_char == ':') {
        break;
    } else if (curr_char == ' ') {
    } else {
        len_param_name++;
    }
  }
  if (len_param_name == 0) {
    return NULL;
  }

  // Inicializacao de parametro
  conf_parameter* param = malloc(sizeof(conf_parameter));

  param->key.value = line;
  param->key.length = len_param_name;

  param->str.length = strlen(line) - len_param_name -1;
  param->str.value = line + len_param_name + 1;

  // Tentar converter parametro para inteiro
  char* end;
  errno = 0;
  long extracted_int = strtoll(param->str.value, &end, 10);
  if (extracted_int != 0L){
    // Não houve nenhum erro na conversão, atualizar o valor do parametro e devolver
    param->i = extracted_int;
  }

  return param;
};

int get_num_of_configuration_parameters_in_file(FILE* iter){
  int count = 0;
  int t = getc(iter);
  if (t == EOF){
    return 0;
  }
  char curr_char = (char)t;
  char prev_char = curr_char;
  while((t = getc(iter)) != EOF){
    curr_char = (char)t;
    if (curr_char == '\n' && prev_char != '\n') {
      count++;
    }
    prev_char = curr_char;
  }
  return count;
}

conf_parameter *get_parameter_from_configuration(configuration *conf,
						 str *parameter_name){
  for (int i = 0; i < conf->num_of_parameters; i++) {
    if(str_equals(&conf->list_of_parameters[i]->key, parameter_name)){
      return conf->list_of_parameters[i];
    }
  }

  return NULL;
}


void free_configuration(configuration* conf){
  for (int i = 0; i < conf->num_of_parameters; i++) {
    conf_parameter* param = conf->list_of_parameters[i];
    str_free(&param->str);
    str_free(&param->key);
  }
}
