#ifndef _THREAD_SYNC_H_
#define _THREAD_SYNC_H_

#include "./mods/utils.h"
#include "./mods/list.h"
#include "./mods/lock.h"
#include "./thread.h"

/**
 * Thread spinlock
 */
struct ThreadSpinLock {

    /* Owner thread */
    Thread owner;

    /* Lock word */
    Lock lock;
};

/**
 * Spinlock members handling
 */
#define spin_set_owner(spin, thread) ((spin)->owner = (thread))
#define spin_get_owner(spin)         ((spin)->owner)
#define spin_acq_lock(spin)          (lock_acquire(&(spin)->lock))
#define spin_rel_lock(spin)          (lock_release(&(spin)->lock))
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
        lock_init(&(spin)->lock);               \
    }
#define spin_free(spin)              (free(spin))

/**
 * Thread mutex
 */
struct ThreadMutex {

    /* Owner thread */
    Thread owner;

    /* Linked list of waiting threads */
    List waitll;

    /* Lock for members */
    Lock mem_lock;
};

/**
 * Mutex members handling
 */
#define mut_set_owner(mut, thread) ((mut)->owner = (thread))
#define mut_get_owner(mut)         ((mut)->owner)
#define mut_has_owner(mut)         ((mut)->owner)
#define mut_lock(mut)              (lock_acquire(&(mut)->mem_lock))
#define mut_unlock(mut)            (lock_release(&(mut)->mem_lock))
#define mut_has_wait_thread(mut)   (!list_is_empty(&(mut)->waitll))
#define mut_add_wait_thread(mut, thread)                \
    (list_enqueue(&(mut)->waitll, (thread), ll_mem))
#define mut_get_wait_thread(mut)                            \
    (list_dequeue(&(mut)->waitll, struct Thread, ll_mem))
#define mut_alloc()                                 \
    ({                                              \
        ThreadMutex __mutex;                        \
                                                    \
        /* Allocate memory */                       \
        __mutex = alloc_mem(struct ThreadMutex);    \
                                                    \
        /* Return the pointer */                    \
        __mutex;                                    \
    })
#define mut_init(mut)                           \
    {                                           \
        /* Set the owner to none */             \
        (mut)->owner = NULL;                    \
                                                \
        /* Initialize the wait list */          \
        list_init(&(mut)->waitll);              \
                                                \
        /* Initialize the lock */               \
        lock_init(&(mut)->mem_lock);            \
    }
#define mut_free(mut)              (free(mut))

#endif
