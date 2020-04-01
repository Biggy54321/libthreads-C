#include <stddef.h>
#include <assert.h>

#include "./hthread_cntl.h"
#include "./hthread_sync.h"

/**
 * @brief Acquires the mutex lock
 *
 * Acquires the mutex lock and sets the owner of the lock to the calling thread.
 * The function does not return unless the lock is acquired
 *
 * @param[in] mutex Pointer to the mutex instance
 */
void hthread_mutex_lock(HThreadMutex *mutex) {

    HThread hthread;

    /* Check for errors */
    assert(mutex);

    /* Get the thread handle */
    hthread = hthread_self();

    /* If the current owner of the lock is the caller thread itself */
    if (mutex->owner == hthread) {

        return;
    }

    /* Acquire the lock */
    lock_acquire(&mutex->lock);

    /* Set the owner to the current thread */
    mutex->owner = hthread;
}

/**
 * @brief Releases the mutex lock
 *
 * Releases the mutex lock and sets the owner of the lock to no one.
 *
 * @param[in] mutex Pointer to the mutex instance
 */
void hthread_mutex_unlock(HThreadMutex *mutex) {

    HThread hthread;

    /* Check for errors */
    assert(mutex);

    /* Get the thread handle */
    hthread = hthread_self();

    /* If the owner of the mutex is not the current thread */
    if (mutex->owner != hthread) {

        return;
    }

    /* Set the owner to no one */
    mutex->owner = NULL;

    /* Release the lock  */
    lock_release(&mutex->lock);
}
