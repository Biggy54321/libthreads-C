#ifndef _THREAD_CNTL_H
#define _THREAD_CNTL_H

#include "./thread_types.h"

int thread_create(
        Thread *thread,
        thread_start_t start_routine,
        ptr_t argument);

int thread_join(
        Thread thread,
        ptr_t *return_value);

Thread thread_self(void);

void thread_exit(ptr_t return_value);

int thread_yield(void);

void thread_main_init(void) __attribute__((constructor));

#endif
