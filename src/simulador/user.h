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
  int age;
  bool deficient;
  bool is_man;
} user_info;

/* Summary: Função onde um utilizador tenta entrar nas atraçòes
   Returns: true se um acidente ocorreu e o user deve sair do parque,
            false se um acidente não ocorreu
 */
bool try_enter_attractions(user_info* info);

void user_entry_point(user_info* info);

void send_message_to_socket(int*socket, MessageType type, char* message);

void thread_send_message_to_socket(int *socket, MessageType type, char *message);

bool is_vip(user_info* info);

#endif
