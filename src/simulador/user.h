#ifndef USER_H
#define USER_H

#include <pthread.h>
#include <bits/pthreadtypes.h>
#include "../common/communication.h"

typedef struct {
  int *socket_monitor;
  int i;
} user_entry_point_info;

void user_entry_point(user_entry_point_info* info);

void send_message_to_socket(int*socket, MessageType type, char* message);

void thread_send_message_to_socket(int *socket, MessageType type, char *message);

#endif
