#ifndef _THREAD_SYNC_H_
#define _THREAD_SYNC_H_

#include "./thread_types.h"

/**
 * Thread Lock definition
 */
typedef struct _ThreadLock {

    /* Owner of the lock */
    Thread owner_thread;

    /* The lock word on which to perform locking */
    uint32_t lock_word;

} ThreadLock;

/* Lock status */
#define THREAD_LOCK_ACQUIRED     (0u)
#define THREAD_LOCK_NOT_ACQUIRED (1u)

/* Lock initializer macro */
#define THREAD_LOCK_INITIALIZER (ThreadLock){NULL, LOCK_NOT_ACQUIRED}

/**
 * Thread mutex definition
 */
typedef ThreadLock ThreadMutex;

/**
 * Thread spinlock definition
 */
typedef ThreadLock ThreadSpinLock;

/* Mutex initializer macro */
#define THREAD_MUTEX_INITIALIZER THREAD_LOCK_INITIALIZER

/* Spinlock initializer macro */
#define THREAD_SPINLOCK_INITIALIZER THREAD_LOCK_INITIALIZER

ThreadReturn thread_mutex_init(ThreadMutex *mutex);

ThreadReturn thread_mutex_lock(ThreadMutex *mutex);

ThreadReturn thread_mutex_unlock(ThreadMutex *mutex);

ThreadReturn thread_spin_init(ThreadSpinLock *spinlock);

ThreadReturn thread_spin_lock(ThreadSpinLock *spinlock);

ThreadReturn thread_spin_unlock(ThreadSpinLock *spinlock);

#endif
