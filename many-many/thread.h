#ifndef _THREAD_H_
#define _THREAD_H_

#include "./types.h"

void thread_lib_init(void);

void thread_create(
        Thread *thread,
        void *(*start_routine)(void *),
        void *argument);

void thread_join(Thread thread, void **return_value);

void thread_exit(void *return_value);

void thread_lock_init(Lock *lock);

void thread_lock(Lock *lock);

void thread_unlock(Lock *lock);

void thread_kill(Thread thread, int sig_num);

Thread thread_self(void);

void thread_lib_deinit(void);

#endif
