#ifndef USER_H
#define USER_H

#include "../common/communication.h"

typedef struct {
  int *socket_monitor;
  int i;
  unsigned long pthread_info;
  int age;
  bool deficient;
  bool is_man;
} user_info;

/* Summary: Função onde um utilizador tenta entrar nas atraçòes
   Returns: true, se um acidente ocorreu e o user deve sair do parque,
            false, se um acidente não ocorreu e o user deve continuar no parque
 */
bool try_enter_attractions(user_info* info);

/* Summary: Entry point de cada user, basicamente o 'main' do user
 */
void user_entry_point(user_info *info);

void send_message_to_socket(int*socket, MessageType type, char* message);

/* Summary: Manda uma mensagem para um determinado socket, respeitando exclusão mútua nesse socket */
void thread_send_message_to_socket(int *socket, MessageType type, char *message);

bool is_vip(user_info* info);

void exit_park(user_info* info);

#endif
