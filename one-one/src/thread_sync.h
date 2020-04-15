#ifndef _THREAD_SYNC_H_
#define _THREAD_SYNC_H_

#include "./mods/utils.h"
#include "./mods/list.h"
#include "./mods/lock.h"
#include "./thread.h"

/**
 * Thread spinlock definition
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

/**
 * Thread mutex definition
 */
struct ThreadMutex {

    /* Owner thread */
    Thread owner;

    /* Padding */
    ptr_t __pad[2];

    /* Lock word */
    Lock lock;
};

/**
 * Mutex statuses
*/
#define MUTEX_ACQUIRED      (0u)
#define MUTEX_NOT_ACQUIRED  (1u)

/**
 * Mutex members handling
 */
#define mut_set_owner(mutex, thread) ((mutex)->owner = (thread))
#define mut_get_owner(mutex)         ((mutex)->owner)
#define mut_acq_lock(mutex)                                             \
    (atomic_cas(&(mutex)->lock, MUTEX_NOT_ACQUIRED, MUTEX_ACQUIRED))
#define mut_rel_lock(mutex)                                             \
    (atomic_cas(&(mutex)->lock, MUTEX_ACQUIRED, MUTEX_NOT_ACQUIRED))
#define mut_wait(mutex)                                 \
    (futex(&(mutex)->lock, FUTEX_WAIT, MUTEX_ACQUIRED))
#define mut_wake(mutex)                         \
    (futex(&(mutex)->lock, FUTEX_WAKE, 1))
#define mut_alloc()                                 \
    ({                                              \
        ThreadMutex __mutex;                        \
                                                    \
        /* Allocate the object */                   \
        __mutex = alloc_mem(struct ThreadMutex);    \
                                                    \
        /* Return the pointer */                    \
        __mutex;                                    \
    })
#define mut_init(mutex)                         \
    {                                           \
        /* Set the owner to none */             \
        (mutex)->owner = NULL;                  \
                                                \
        /* Initialize the lock word */          \
        (mutex)->lock = MUTEX_NOT_ACQUIRED;     \
    }
#define mut_free(mutex)              (free(mutex))

#endif
