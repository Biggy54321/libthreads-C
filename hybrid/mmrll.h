#ifndef _MMRLL_H_
#define _MMRLL_H_

#include "./thread.h"

void mmrll_init(void);

Thread mmrll_dequeue(void);

void mmrll_enqueue(Thread thread);

int mmrll_is_empty(void);

void mmrll_lock(void);

void mmrll_unlock(void);

#endif
