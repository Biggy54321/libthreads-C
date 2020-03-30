#ifndef _MM_RDY_LIST_H_
#define _MM_RDY_LIST_H_

#include "./hthread_pub.h"

void mm_rdy_list_add(HThread hthread);

HThread mm_rdy_list_get(void);

int mm_rdy_list_is_empty(void);

void mm_rdy_list_lock(void);

void mm_rdy_list_unlock(void);

#endif
