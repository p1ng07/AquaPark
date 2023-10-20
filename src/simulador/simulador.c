#include "../common/configuration.h"
#include "../common/string.h"
#include "print.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {

  if (argc < 2){
    printf("ERRO: Não foi especificado um ficheiro de configuração");
    return 1;
  }

  configuration conf = extract_config_from_file(argv[1]);

  conf_parameter *param = get_parameter_from_configuration(
      &conf, new_str("negativo"));

  if(param != NULL){
    printf("%d", param->i);
  }

  return 0;
}
