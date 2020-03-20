#ifndef _HTHREAD_LIST_H_
#define _HTHREAD_LIST_H_

#include "./hthread_defs.h"

void hthread_list_init(void);

void hthread_list_add(HThread hthread);

HThread hthread_list_get(void);

int hthread_list_is_empty(void);

void hthread_list_lock(void);

void hthread_list_unlock(void);

#endif
