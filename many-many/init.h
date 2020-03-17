#ifndef _INIT_H_
#define _INIT_H_

#include "./types.h"

void init_thread(Thread *thread, void *(*start_routine)(void *), void *argument);

#endif
