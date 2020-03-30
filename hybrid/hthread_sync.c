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

    /* Check for errors */
    assert(mutex);

    /* Acquire the lock */
    lock_acquire(&mutex->lock);

    /* Set the owner to the current thread */
    mutex->owner = hthread_self();
}

/**
 * @brief Releases the mutex lock
 *
 * Releases the mutex lock and sets the owner of the lock to no one.
 *
 * @param[in] mutex Pointer to the mutex instance
 */
void hthread_mutex_unlock(HThreadMutex *mutex) {

    /* Check for errors */
    assert(mutex);

    /* Set the owner to no one */
    mutex->owner = NULL;

    /* Release the lock  */
    lock_release(&mutex->lock);
}
