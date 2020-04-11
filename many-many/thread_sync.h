#ifndef _THREAD_SYNC_H_
#define _THREAD_SYNC_H_

#include "./mods/list.h"
#include "./mods/lock.h"
#include "./thread.h"

/**
 * Thread spinlock
*/
struct ThreadSpinLock {

    /* Owner thread */
    Thread owner;

    /* Lock word */
    Lock lock;
};

/**
 * Thread mutex
 */
struct ThreadMutex {

    /* Owner thread */
    Thread owner;

    /* List of thread competing for the mutex */
    List waitll;

    /* Lock for members */
    Lock mem_lock;
};

/* Spinlock status */
#define THREAD_SPINLOCK_ACQUIRED     (LOCK_ACQUIRED)
#define THREAD_SPINLOCK_NOT_ACQUIRED (LOCK_NOT_ACQUIRED)

#endif
