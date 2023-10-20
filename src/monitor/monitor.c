#include "../common/configuration.h"
#include "events.h"
#include <stdio.h>

int main(int argc, char *argv[]) {

  if (argc < 3){
    printf("ERRO: Têm de ser especificados ficheiros de configuração e de escrita de eventos, nessa ordem");
    return 1;
  }

  // Carregar configuração de ficheiro
  configuration _conf = extract_config_from_file(argv[1]);


  // File handler com opção "append" (leitura, escrita, juntar ao fim do ficheiro)
  FILE* file_eventos = fopen(argv[2], "a");

  write_event(file_eventos, "O Julio é o Cbum, %s \n", "e o miguel é o ramon");
  
  return 0;
}
