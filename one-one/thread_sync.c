#include <errno.h>

#include "./mods/utils.h"
#include "./thread_cntl.h"
#include "./thread_sync.h"

/**
 * Number of processes to wake up after the lock is released
 */
#define NB_WAKEUP_PROCESSES (1)

/**
 * @brief Acquires the spinlock
 * @param[in/out] spinlock Pointer to the spinlock instance
 * @note The call is blocking and will return only if the lock is acquired
 */
ThreadReturn thread_spinlock(ThreadSpinLock *spinlock) {

    Thread thread;
    int ret_val;

    /* Check for errors */
    if (!spinlock) {

        return THREAD_FAIL;
    }

    /* Get the thread handle */
    thread = thread_self();

    /* Check if the current thread already owns the spinlock */
    if (spinlock->owner_thread == thread) {

        return THREAD_OK;
    }

    /* For eternity */
    while (1) {

        /* Atomically try to acquire the lock */
        if (atomic_cas(&spinlock->lock_word,
                       SPINLOCK_NOT_ACQUIRED,
                       SPINLOCK_ACQUIRED)) {

            /* Set the owner to the current thread */
            spinlock->owner_thread = thread;

            break;
        }

        /* Wait till the lock is not released by the current owner */
        ret_val = futex(&spinlock->lock_word, FUTEX_WAIT, SPINLOCK_ACQUIRED);

        /* Check for errors */
        if ((ret_val == -1) && (errno != EAGAIN)) {

            return THREAD_FAIL;
        }
    }

    return THREAD_OK;
}

/**
 * @brief Releases the spinlock
 * @param[in/out] spinlock Pointer to the spinlock instance
 */
ThreadReturn thread_spinunlock(ThreadSpinLock *spinlock) {

    Thread thread;
    int ret_val;

    /* Check for errors */
    if (!spinlock) {

        return THREAD_FAIL;
    }

    /* Get the thread handle */
    thread = thread_self();

    /* If the current thread is not the owner */
    if (spinlock->owner_thread != thread) {

        /* Just return */
        return THREAD_OK;
    }

    /* Set the owner to no one */
    spinlock->owner_thread = NULL;

    /* Release the lock atomically */
    if (atomic_cas(&spinlock->lock_word,
                   SPINLOCK_ACQUIRED,
                   SPINLOCK_NOT_ACQUIRED)) {

        /* Wake up the waiting process */
        ret_val = futex(&spinlock->lock_word, FUTEX_WAKE, NB_WAKEUP_PROCESSES);

        /* Check for errors */
        if ((ret_val == -1) && (errno != EAGAIN)) {

            return THREAD_FAIL;
        }
    }

    return THREAD_OK;
}
