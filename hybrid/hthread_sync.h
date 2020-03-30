#ifndef _HTHREAD_SYNC_H_
#define _HTHREAD_SYNC_H_

#include "./mods/lock.h"
#include "./hthread_pub.h"

/**
 * Thread mutex lock
*/
typedef struct HThreadMutex {

    /* Owner thread of the lock */
    HThread owner;

    /* Lock word */
    Lock lock;

} HThreadMutex;

/* Thread mutex initializer */
#define HTHREAD_MUTEX_INITIALIZER {NULL, LOCK_NOT_ACQUIRED}

void hthread_mutex_lock(HThreadMutex *mutex);

void hthread_mutex_unlock(HThreadMutex *mutex);

#endif
