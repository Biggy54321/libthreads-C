#include <stdatomic.h>

#include "./lock.h"

/**
 * @brief Atomic compare and swap
 * @param[in] addr Address of lock variable
 * @param[in] old_val Old value expected in the lock variable
 * @param[in] new_value New value to be set in the lock variable
 * @return 1 if the value at addr is updated
 * @return 0 if the value at addr is not updated
 */
static inline int _atomic_cas(int *addr, int old_val, int new_val) {

    /* Use the atomic function to compare and exchange */
    return atomic_compare_exchange_strong(addr, &old_val, new_val);
}


/**
 * @brief Locks the given lock variable
 * @param[in] lock Pointer to the lock variable
 */
void lock_acquire(Lock *lock) {

    /* Check for errors */
    assert(lock);

    /* While the lock's status is not updated */
    while (!_atomic_cas(lock, LOCK_NOT_ACQUIRED, LOCK_ACQUIRED));
}

/**
 * @brief Unlocks the given lock variable
 * @param[in] lock Pointer to the lock variable
 */
void lock_release(Lock *lock) {

    /* Check for errors */
    assert(lock);

    /* If the lock is updated with the release value */
    _atomic_cas(lock, LOCK_ACQUIRED, LOCK_NOT_ACQUIRED);
}
