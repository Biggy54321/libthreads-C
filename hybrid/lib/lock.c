#include <assert.h>

#include "./utils.h"
#include "./lock.h"

/**
 * @brief Initialize the lock
 *
 * Sets the lock status to not acquired
 *
 * @param[in] Pointer to the lock variable
 */
void lock_init(Lock *lock) {

    /* Set the status to not acquired */
    *lock = LOCK_NOT_ACQUIRED;
}

/**
 * @brief Acquire the lock
 *
 * Atomically checks if the lock is not acquired and then acquires it. Busy
 * waits till he lock is not acquired
 *
 * @param[in] lock Pointer to the lock variable
 */
void lock_acquire(Lock *lock) {

    /* Check for errors */
    assert(lock);

    /* While the lock's status is not updated */
    while (!atomic_cas(lock, LOCK_NOT_ACQUIRED, LOCK_ACQUIRED));
}

/**
 * @brief Releases the lock
 *
 * Atomically check if the lock is acquired and then releases it
 *
 * @param[in] lock Pointer to the lock variable
 */
void lock_release(Lock *lock) {

    /* Check for errors */
    assert(lock);

    /* If the lock is updated with the release value */
    atomic_cas(lock, LOCK_ACQUIRED, LOCK_NOT_ACQUIRED);
}
