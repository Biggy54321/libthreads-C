#include <stddef.h>
#include <assert.h>

#include "./hthread_cntl.h"
#include "./hthread_sync.h"

/**
 * @brief Acquires the spinlock
 *
 * Acquires the spinlock and sets the owner of the lock to the calling thread.
 * The function does not return unless the lock is acquired
 *
 * @param[in] spinlock Pointer to the spinlock instance
 */
void hthread_spinlock_lock(HThreadMutex *spinlock) {

    HThread hthread;

    /* Check for errors */
    assert(spinlock);

    /* Get the thread handle */
    hthread = hthread_self();

    /* If the current owner of the lock is the caller thread itself */
    if (spinlock->owner == hthread) {

        return;
    }

    /* Acquire the lock */
    lock_acquire(&spinlock->lock);

    /* Set the owner to the current thread */
    spinlock->owner = hthread;
}

/**
 * @brief Releases the spinlock
 *
 * Releases the spinlock and sets the owner of the lock to no one.
 *
 * @param[in] spinlock Pointer to the spinlock instance
 */
void hthread_spin_unlock(HThreadMutex *spinlock) {

    HThread hthread;

    /* Check for errors */
    assert(spinlock);

    /* Get the thread handle */
    hthread = hthread_self();

    /* If the owner of the spinlock is not the current thread */
    if (spinlock->owner != hthread) {

        return;
    }

    /* Set the owner to no one */
    spinlock->owner = NULL;

    /* Release the lock  */
    lock_release(&spinlock->lock);
}
