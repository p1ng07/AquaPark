#ifndef USER_H
#define USER_H

#include <pthread.h>
#include <stdbool.h>
#include <bits/pthreadtypes.h>
#include "../common/communication.h"

typedef struct {
  int *socket_monitor;
  int i;
  unsigned long pthread_info;
  int idade;
  bool deficient;
  bool is_man;
} user_entry_point_info;

/* Summary: Função onde um utilizador tenta entrar nas atraçòes
   Returns: true se um acidente ocorreu e o user deve sair do parque,
            false se um acidente não ocorreu
 */
bool try_enter_attractions(user_entry_point_info* info);

void user_entry_point(user_entry_point_info* info);

void send_message_to_socket(int*socket, MessageType type, char* message);

void thread_send_message_to_socket(int *socket, MessageType type, char *message);

#endif
