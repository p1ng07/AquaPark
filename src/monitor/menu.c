#include <stdio.h>

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

void clean_screen() { printf("\x1B[2J\x1B[1;1H"); }
