#ifndef PISCINA_H
#define PISCINA_H

#include "user.h"
#include "../common/common.h"

/* Summary: Tenta entrar na piscina
   A piscina não tem fila, mas tem um limite máximo de 40 pessoas.
   Se alguém tentar entrar e não houver espaço, deve só sair por onde veio e não
   ficar há espera

   info: Informação sobre o user corrente

   @Returns: True, se houve um acidente
             False, se não houve acidente
*/
bool piscina(user_info*);

#endif
