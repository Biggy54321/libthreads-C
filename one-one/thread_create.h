#ifndef _THREAD_CREATE_H_
#define _THREAD_CREATE_H_

#include "./thread_types.h"

/**
 * Thread create signature
 */
ThreadReturn thread_create(
        Thread *thread,
        thread_start_t start_routine,
        ptr_t argument);

#endif
