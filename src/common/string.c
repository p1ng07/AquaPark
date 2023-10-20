#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string.h"

void str_print(str string){
  for (int i = 0; i < string.length; i++) {
    printf("%c", string.value[i]);
  }
  printf("\n");
}

int str_equals(str* str1, str* str2){
  if (str1->length != str2->length){
    return 0;
  }

  for (int i = 0; i < str1->length; i++) {
    if (str1->value[i] != str2->value[i]) {
      return 0;
    }
  }

  return 1;
}

str* new_str(char* null_term_string){
  str* string = malloc(sizeof(str));

  char *heap_allocated_string = malloc(sizeof(char) * strlen(null_term_string));
  strcpy(heap_allocated_string, null_term_string);

  string->length = strlen(null_term_string);
  string->value = heap_allocated_string;
  return string;
}

void str_free(str* str){
  free(str->value);
  free(str);
}

