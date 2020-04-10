#include <stdlib.h>
#include <stddef.h>
#include <assert.h>

#include "./mods/utils.h"
#include "./thread_descr.h"
#include "./thread_sync.h"

/**
 * @brief Initializes the spinlock
 *
 * Allocates the required memory for the spinlock object and initializes it
 * to the base value
 *
 * @param[in] spinlock Pointer to the spinlock instance
 */
void thread_spin_init(ThreadSpinLock *spinlock) {

    /* Check for errors */
    assert(spinlock);

    /* Allocate the memory */
    (*spinlock) = alloc_mem(struct ThreadSpinLock);

    /* Set the owner to none */
    (*spinlock)->owner = NULL;

    /* Set the status to not acquired */
    (*spinlock)->lock = THREAD_SPINLOCK_NOT_ACQUIRED;
}

/**
 * @brief Acquires the spinlock
 *
 * Acquires the spinlock and sets the owner of the lock to the calling thread.
 * The function does not return unless the lock is acquired
 *
 * @param[in] spinlock Pointer to the spinlock instance
 */
void thread_spin_lock(ThreadSpinLock *spinlock) {

    Thread thread;

    /* Check for errors */
    assert(spinlock);
    assert(*spinlock);

    /* Get the thread handle */
    thread = thread_self();

    /* If the current thread is the owner */
    if ((*spinlock)->owner == thread) {

        return;
    }

    /* Update the state */
    TD_SET_STATE(thread, THREAD_STATE_WAIT_SPINLOCK);

    /* Acquire the lock */
    lock_acquire(&(*spinlock)->lock);

    /* Update the state */
    TD_SET_STATE(thread, THREAD_STATE_RUNNING);

    /* Set the owner to the current thread */
    (*spinlock)->owner = thread;
}

/**
 * @brief Releases the spinlock
 *
 * Releases the spinlock and sets the owner of the lock to no one.
 *
 * @param[in] spinlock Pointer to the spinlock instance
 */
void thread_spin_unlock(ThreadSpinLock *spinlock) {

    Thread thread;

    /* Check for errors */
    assert(spinlock);
    assert(*spinlock);

    /* Get the thread handle */
    thread = thread_self();

    /* If the owner of the spinlock is not the current thread */
    if ((*spinlock)->owner != thread) {

        return;
    }

    /* Set the owner to none */
    (*spinlock)->owner = NULL;

    /* Release the lock */
    lock_release(&(*spinlock)->lock);
}

/**
 * @brief Destroy the spinlock
 *
 * Free the allocated memory for the spinlock object
 *
 * @param[in] spinlock Pointer to the spinlock instance
 */
void thread_spin_destroy(ThreadSpinLock *spinlock) {

    /* Check for errors */
    assert(spinlock);
    assert(*spinlock);

    /* Free the allocated memory */
    free(*spinlock);
}
