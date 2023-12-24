#include "bathrooms.h"
#include <sys/queue.h>

pthread_mutex_t deficient_bathroom_lock = PTHREAD_MUTEX_INITIALIZER;

SLIST_HEAD(deficient_restroom_queue, sem_t) restroom_queue = SLIST_HEAD_INITIALIZER(restroom_queue);

void enter_bathrooms(user_info* info){
  if(info->deficient){
    // casa de banho individual dos deficientes
  }else if (info->is_man){
    // Casas de banho duplas de homens
  }else if (!info->is_man){
    // Casas de banho duplas de mulheres
  }
}
