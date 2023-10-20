#include <stdio.h>
#include "string.h"

void str_print(str string){
  for (int i = 0; i < string.length; i++) {
    printf("%c", string.value[i]);
  }
  printf("\n");
}
