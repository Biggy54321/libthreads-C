#include <stdatomic.h>

#include "./spin_lock.h"

/**
 * @brief Atomic compare and swap macro
 * @param[in] addr Address of lock variable
 * @param[in] old_val Old value expected in the lock variable
 * @param[in] new_value New value to be set in the lock variable
 * @return 1 if the value at addr is updated
 * @return 0 if the value at addr is not updated
 */
#define _ATOM_CAS(addr, old_val, new_val)                               \
    ({                                                                  \
        Lock _old_val = (old_val);                                      \
                                                                        \
        atomic_compare_exchange_strong((addr), &_old_val, (new_val));   \
    })

/**
 * @brief Locks the given lock variable
 * @param[in] lock Pointer to the lock variable
 */
void spin_lock(Lock *lock) {

    /* While the lock's status is not updated */
    while (!_ATOM_CAS(lock, SPIN_LOCK_NOT_TAKEN, SPIN_LOCK_TAKEN));
}

/**
 * @brief Unlocks the given lock variable
 * @param[in] lock Pointer to the lock variable
 */
void spin_unlock(Lock *lock) {

    /* If the lock is updated with the release value */
    _ATOM_CAS(lock, SPIN_LOCK_TAKEN, SPIN_LOCK_NOT_TAKEN);
}
