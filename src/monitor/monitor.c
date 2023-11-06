#include "../common/configuration.h"
#include "events.h"
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>

/*
  Summary: Retorna um input numérico válido num
    determinado intervalo (numero entre min_choice e max_choice)
 */
int get_menu_input(int min_choice, int max_choice) {
  int input_choice = -1;
  int result = scanf("%d", &input_choice);

  if (result == EOF) {
    // Houve algum erro com stdio
    input_choice = -1;
  }

  if (result == 0) {
    // Input não foi válido, fazer flush de stdin
    printf("\nInput inválido;\n");
    input_choice = -1;
    while (fgetc(stdin) != '\n')
      ; // Read until a newline is found
  }

  if (input_choice > max_choice || input_choice < min_choice) {
    // Input tem de estar num determinado intervalo de numeros
    input_choice = -1;
  }
  return input_choice;
}

int main(int argc, char *argv[]) {

  if (argc < 3){
    perror("ERRO: Têm de ser especificados ficheiros de configuração e de escrita de eventos, nessa ordem");
    return 1;
  }

  // Carregar configuração de ficheiro
  configuration _conf = extract_config_from_file(argv[1]);


  // File handler com opção "append" (leitura, escrita, juntar ao fim do ficheiro)
  FILE* file_eventos = fopen(argv[2], "a");

  bool menu_principal_running = true;

  while (menu_principal_running) {
    printf("\x1B[2J\x1B[1;1H");
    printf("1 - Iniciar simulação\n");
    printf("2 - Terminar simulação\n");
    // TODO: Adicionar opcoes de passar horas quando a simulação está a correr
    printf("3 - Reiniciar ficheiro de eventos\n");
    printf("4 - Passar hora na simulação\n");
    printf("5 - Passar x horas na simulação\n");
    printf("Opcao: ");

    int input_choice = get_menu_input(0, 4);

    switch (input_choice) {
    case 1:
      // TODO Iniciar simulação
      break;
    case 2:
      // TODO Terminar simulaçã̀o
      break;
    case 3:
      // TODO Limpar ficheiro de eventos

      // Reabrir o ficheiro em modo de escrita faz com que os conteúdos sejam apagados
      freopen(argv[2], "w", file_eventos);
      freopen(argv[2], "a", file_eventos);
      break;
    case 4:
      // TODO Mandar sinal para passar uma hora na simulação
      break;
    case 5:
      // TODO Mandar sinal para passar x horas na simulação
      printf("\n Quantas horas devem passar? ");
      int elapsed_hours = get_menu_input(0, INT_MAX);
      break;
    }
  }

  write_event(file_eventos, "O Julio é o Cbum, %s \n", "e o miguel é o ramon");
  
  return 0;
}
