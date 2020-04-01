#ifndef _THREAD_CNTL_H
#define _THREAD_CNTL_H

#include "./thread_types.h"

ThreadReturn thread_create(
        Thread *thread,
        thread_start_t start_routine,
        ptr_t argument);

ThreadReturn thread_join(
        Thread thread,
        ptr_t *return_value);

Thread thread_self(void);

void thread_exit(ptr_t return_value);

void thread_main_init(void) __attribute__((constructor));

#endif
