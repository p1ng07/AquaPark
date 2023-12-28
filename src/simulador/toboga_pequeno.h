#ifndef TOBOGA_PEQUENO_H
#define TOBOGA_PEQUENO_H

#include "user.h"

/* Summary: Entry point para o worker do tobogan pequeno
   O worker vai trabalhar (libertar users na fila de espera, por ordem de
   chegada, vips primeiro) enquanto o parque estiver aberto.
   Assim que o parque fechar (controlado pela variável global parque_aberto),
   todos os utilizadores saem da fila e do parque.
 */
void tobogan_pequeno_worker_entry_point();

/* Summary: Tenta entrar no tobogan pequeno
   O tobogan tem uma fila de espera que respeita prioridades(i.e tenta deixar
   passar users vips antes de deixar passar users normais)

   info: Informação sobre o user corrente

   @Returns: True, se houve um acidente
             False, se não houve acidente
*/
bool tobogan_pequeno(user_info *info);

#endif
