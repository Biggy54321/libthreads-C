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
#define SPIN_SET_OWNER(spin, thread) ((spin)->owner = (thread))
#define SPIN_GET_OWNER(spin)         ((spin)->owner)
#define SPIN_ACQ_LOCK(spin)          (lock_acquire(&(spin)->lock))
#define SPIN_REL_LOCK(spin)          (lock_release(&(spin)->lock))
#define SPIN_ALLOC()                                \
    ({                                              \
        ThreadSpinLock __spin;                      \
                                                    \
        /* Allocate the object */                   \
        __spin = alloc_mem(struct ThreadSpinLock);  \
                                                    \
        /* Return the pointer */                    \
        __spin;                                     \
    })
#define SPIN_INIT(spin)                         \
    {                                           \
        /* Set the owner to none */             \
        (spin)->owner = NULL;                   \
                                                \
        /* Initialize the lock word */          \
        lock_init(&(spin)->lock);               \
    }
#define SPIN_FREE(spin)              (free(spin))

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
#define MUT_SET_OWNER(mut, thread) ((mut)->owner = (thread))
#define MUT_GET_OWNER(mut)         ((mut)->owner)
#define MUT_HAS_OWNER(mut)         ((mut)->owner)
#define MUT_LOCK(mut)              (lock_acquire(&(mut)->mem_lock))
#define MUT_UNLOCK(mut)            (lock_release(&(mut)->mem_lock))
#define MUT_HAS_WAIT_THREAD(mut)   (!list_is_empty(&(mut)->waitll))
#define MUT_ADD_WAIT_THREAD(mut, thread)                \
    (list_enqueue(&(mut)->waitll, (thread), ll_mem))
#define MUT_GET_WAIT_THREAD(mut)                            \
    (list_dequeue(&(mut)->waitll, struct Thread, ll_mem))
#define MUT_ALLOC()                                 \
    ({                                              \
        ThreadMutex __mutex;                        \
                                                    \
        /* Allocate memory */                       \
        __mutex = alloc_mem(struct ThreadMutex);    \
                                                    \
        /* Return the pointer */                    \
        __mutex;                                    \
    })
#define MUT_INIT(mut)                           \
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
#define MUT_FREE(mut)              (free(mut))

#endif
