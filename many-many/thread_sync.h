#ifndef _THREAD_SYNC_H_
#define _THREAD_SYNC_H_

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

/* Spinlock status */
#define THREAD_SPINLOCK_ACQUIRED     (LOCK_ACQUIRED)
#define THREAD_SPINLOCK_NOT_ACQUIRED (LOCK_NOT_ACQUIRED)

#endif
