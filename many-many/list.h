#ifndef _LIST_H_
#define _LIST_H_

#include "./types.h"

void list_enqueue(Thread thread);

Thread list_dequeue(void);

int list_is_empty(void);

void list_lock(void);

void list_unlock(void);

#endif
