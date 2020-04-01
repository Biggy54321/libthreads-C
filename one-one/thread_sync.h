#ifndef _THREAD_SYNC_H_
#define _THREAD_SYNC_H_

#include "./thread_types.h"

/**
 * Thread mutex definition
 */
typedef struct _ThreadMutex {

    /* Owner of the lock */
    Thread owner_thread;

    /* The lock word on which to perform locking */
    uint32_t lock_word;

} ThreadMutex;

/* Mutex status */
#define MUTEX_ACQUIRED     (0u)
#define MUTEX_NOT_ACQUIRED (1u)

/* Mutex initializer macro */
#define THREAD_MUTEX_INITIALIZER (ThreadMutex){NULL, MUTEX_NOT_ACQUIRED}

ThreadReturn thread_mutex_init(ThreadMutex *mutex);

ThreadReturn thread_mutex_lock(ThreadMutex *mutex);

ThreadReturn thread_mutex_unlock(ThreadMutex *mutex);

#endif
