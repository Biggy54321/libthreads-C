#ifndef _USER_THREADS_LIST_H_
#define _USER_THREADS_LIST_H_

#include "./user_thread.h"

void list_add_thread(UserThread user_thread);

UserThread list_get_thread(void);

int list_is_empty(void);

void list_lock(void);

void list_unlock(void);

#endif
