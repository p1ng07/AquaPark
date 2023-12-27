#ifndef BATHROOMS_H
#define BATHROOMS_H

#include "user.h"
#include <semaphore.h>
#include <sys/queue.h>
#include <stdbool.h>

// Estado que indica o que o utilizador deve fazer assim que sair da atração
typedef enum {
  RUNNING, // User deve continuar dentro do parque
  QUIT, // User desistiu da sua fila de espera
  ACCIDENT, // User teve um acidente e tem que sair do parque
}user_state ;

/* Summary: O user tenta entrar na sua casa de banho respetiva.
   Vai para a lista de espera respetiva e espera que o processo "worker" da
   casa de banho o faça avançar.

   As filas de espera das casas de banho:
   - Não tem prioridades.
   - Tem desistências justas (sempre que um user entra na atração, é rolada uma
   chance para cada user desistir da fila de espera caso esta tenha 2 ou mais
   users).

   @Returns:
   True, se ocorreu um acidente e o user deve sair do parque de diversões
   False, se não ocorreu um acidente (o user usou a atração ou desistiu da fila
   de espera) e deve continuar no parque
 */
bool enter_bathrooms(user_info *info);

/* Summary: Entry point para o worker da casa de banho de deficientes.
   O worker vai trabalhar (libertar users na fila de espera) enquanto o parque
   estiver aberto.
   Assim que o parque fechar (controlado pela variável global parque_aberto),
   todos os utilizadores (por ordem da fila) desistem da fila de espera
 */
void disabled_bathroom_worker_entry_point();

/* Summary: Tenta entrar na casa de banho dos deficientes

   info: Informação sobre o user corrente

   @Returns: True, se houve um acidente
             False, se não houve acidente
*/
bool disabled_wc(user_info *info);

/* Summary: Tenta entrar na casa de banho dos deficientes

   info: Informação sobre o user corrente

   @Returns: True, se houve um acidente
             False, se não houve acidente
*/
bool men_wc(user_info* info);

/* Summary: Entry point para o worker da casa de banho de deficientes.
   O worker vai trabalhar (libertar users na fila de espera) enquanto o parque
   estiver aberto.
   Podem estar duas pessoas ao mesmo tempo nas casas de banho
   As casas de banho respeitam prioridades (tenta libertar todos os vips antes de libertar pessoas normais)
   Assim que o parque fechar (controlado pela variável global parque_aberto),
   todos os utilizadores (por ordem da fila) desistem da fila de espera
 */
void men_bathroom_worker_entry_point();
#endif
