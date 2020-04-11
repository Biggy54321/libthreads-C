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

/**
 * Condition variable definition
 */
typedef struct _ThreadCond {

    /* Number of threads waiting  */
    int nb_threads;

    /* Pointer to the mutex linked with */
    ThreadMutex *mutex;

    /* Wait word */
    int wait;

} ThreadCond;

/* Condition variable initializer */
#define THREAD_CONDITION_INITIALIZER (ThreadCond){0, NULL, 0}

int thread_mutex_init(ThreadMutex *mutex);

int thread_mutex_lock(ThreadMutex *mutex);

int thread_mutex_unlock(ThreadMutex *mutex);

int thread_spin_init(ThreadSpinLock *spinlock);

int thread_spin_lock(ThreadSpinLock *spinlock);

int thread_spin_unlock(ThreadSpinLock *spinlock);

int thread_cond_init(ThreadCond *cond);

int thread_cond_wait(ThreadCond *cond, ThreadMutex *mutex);

int thread_cond_signal(ThreadCond *cond);

int thread_cond_broadcast(ThreadCond *cond);

#endif
