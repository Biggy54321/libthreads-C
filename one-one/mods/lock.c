#include <assert.h>

#include "./utils.h"
#include "./lock.h"

/**
 * @brief Acquire the lock
 *
 * Atomically checks if the lock is not acquired and then acquires it. Busy
 * waits till he lock is not acquired
 *
 * @param[in] lock Pointer to the lock variable
 */
int lock_acquire(Lock *lock) {

    /* Check for errors */
    if (!lock) {

        return -1;
    }

    /* While the lock's status is not updated */
    while (!atomic_cas(lock, LOCK_NOT_ACQUIRED, LOCK_ACQUIRED));

    return 0;
}

/**
 * @brief Releases the lock
 *
 * Atomically check if the lock is acquired and then releases it
 *
 * @param[in] lock Pointer to the lock variable
 */
int lock_release(Lock *lock) {

    /* Check for errors */
    if (!lock) {

        return -1;
    }

    /* If the lock is updated with the release value */
    atomic_cas(lock, LOCK_ACQUIRED, LOCK_NOT_ACQUIRED);

    return 0;
}
