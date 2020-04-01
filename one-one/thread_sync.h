#ifndef _THREAD_SYNC_H_
#define _THREAD_SYNC_H_

#include "./thread_types.h"

/**
 * Thread spinlocks definition
 */
typedef struct _ThreadSpinLock {

    /* Owner of the lock */
    Thread owner_thread;

    /* The lock word on which to perform locking */
    uint32_t lock_word;

} ThreadSpinLock;

/* Spinlock status */
#define SPINLOCK_ACQUIRED     (0u)
#define SPINLOCK_NOT_ACQUIRED (1u)

/* Spinlock initializer macro */
#define THREAD_SPINLOCK_INITIALIZER (ThreadSpinLock){NULL, SPINLOCK_NOT_ACQUIRED}

ThreadReturn thread_spinlock(ThreadSpinLock *spinlock);

ThreadReturn thread_spinunlock(ThreadSpinLock *spinlock);

#endif
