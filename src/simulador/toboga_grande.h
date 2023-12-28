#ifndef TOBOGA_GRANDE_H
#define TOBOGA_GRANDE_H

#include "user.h"

/* Summary: Entry point para o worker do tobogan grande
   
   A cada 5 ms, esta thread vai tentar libertar dois users das filas de espera.
   Ao libertá-los, estes entrarão na atração ao mesmo tempo e sairam ao mesmo
   tempo. O worker tenta sempre libertar users vips antes de libertar normais.
   Se não existirem dois users diferentes (vips ou normais), apenas um entrará na
   atração

   Sempre que uma viagem é completa, o worker informa a thread de comunicação
   do tipo da viagem, se houve um ou dois utilizadores

   Assim que o parque fechar (controlado pela variável global parque_aberto),
   todos os utilizadores saem da fila e do parque.
 */
void tobogan_grande_worker_entry_point(int* communication_socket);

/* Summary: Tenta entrar no tobogan grande
   O tobogan tem uma fila de espera que respeita prioridades(i.e tenta deixar
   passar users vips antes de deixar passar users normais)

   info: Informação sobre o user corrente

   @Returns: True, se houve um acidente
             False, se não houve acidente
*/
bool tobogan_grande(user_info *info);

#endif
