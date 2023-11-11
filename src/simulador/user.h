#ifndef USER_H
#define USER_H

#include <pthread.h>
#include <bits/pthreadtypes.h>

typedef struct {
  int *socket_monitor;
  int i;
} user_entry_point_info;

void user_entry_point(user_entry_point_info *info);

void send_string_to_monitor(int* socket_monitor, char* mensagem);

#endif
