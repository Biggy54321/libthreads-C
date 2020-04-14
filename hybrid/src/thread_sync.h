#ifndef _THREAD_SYNC_H_
#define _THREAD_SYNC_H_

#include "./mods/lock.h"
#include "./thread.h"

/**
 * Thread spinlock
*/
struct ThreadSpinLock {

    /* Owner thread */
    Thread owner;

    /* Lock word */
    int lock;
};

/**
 * Spinlock statuses
*/
#define SPINLOCK_ACQUIRED      (0u)
#define SPINLOCK_NOT_ACQUIRED  (1u)

/**
 * Spinlock members handling
 */
#define spin_set_owner(spin, thread) ((spin)->owner = (thread))
#define spin_get_owner(spin)         ((spin)->owner)
#define spin_acq_lock(spin)                                             \
    (atomic_cas(&(spin)->lock, SPINLOCK_NOT_ACQUIRED, SPINLOCK_ACQUIRED))
#define spin_rel_lock(spin)                                             \
    (atomic_cas(&(spin)->lock, SPINLOCK_ACQUIRED, SPINLOCK_NOT_ACQUIRED))
#define spin_alloc()                                \
    ({                                              \
        ThreadSpinLock __spin;                      \
                                                    \
        /* Allocate the object */                   \
        __spin = alloc_mem(struct ThreadSpinLock);  \
                                                    \
        /* Return the pointer */                    \
        __spin;                                     \
    })
#define spin_init(spin)                         \
    {                                           \
        /* Set the owner to none */             \
        (spin)->owner = NULL;                   \
                                                \
        /* Initialize the lock word */          \
        (spin)->lock = SPINLOCK_NOT_ACQUIRED;   \
    }
#define spin_free(spin)              (free(spin))

#endif
