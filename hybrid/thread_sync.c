#include <stddef.h>
#include <assert.h>

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

    /* Allocate the spinlock */
    *spinlock = spin_alloc();

    /* Initialize the spinlock */
    spin_init(*spinlock);
}

/**
 * @brief Acquires the spinlock
 *
 * Acquires the spinlock. The calling thread will busy wait till the lock is
 * not acquired, i.e. the thread will not be blocked and will consume CPU
 * while waiting
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
    if (spin_get_owner(*spinlock) == thread) {

        return;
    }

    /* Acquire the lock */
    spin_acq_lock(*spinlock);

    /* Set the owner to the current thread */
    spin_set_owner(*spinlock, thread);
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
    if (spin_get_owner(*spinlock) != thread) {

        return;
    }

    /* Set the owner to none */
    spin_set_owner(*spinlock, NULL);

    /* Release the lock */
    spin_rel_lock(*spinlock);
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
    spin_free(*spinlock);
}
