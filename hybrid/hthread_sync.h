#ifndef _HTHREAD_SYNC_H_
#define _HTHREAD_SYNC_H_

#include "./mods/lock.h"
#include "./hthread_pub.h"

/**
 * Thread spinlock
*/
typedef struct HThreadSpinLock {

    /* Owner thread of the lock */
    HThread owner;

    /* Lock word */
    Lock lock;

} HThreadSpinLock;

/* Thread spinlock initializer */
#define HTHREAD_SPINLOCK_INITIALIZER (HThreadSpinLock){NULL, LOCK_NOT_ACQUIRED}

void hthread_spin_lock(HThreadSpinLock *spinlock);

void hthread_spin_unlock(HThreadSpinLock *spinlock);

#endif
