#include <errno.h>

#include "./mods/utils.h"
#include "./thread_cntl.h"
#include "./thread_sync.h"

/**
 * Number of processes to wake up after the lock is released
 */
#define NB_WAKEUP_PROCESSES (1)

/**
 * @brief Initialize the mutex
 * @param[in] mutex Pointer to the mutex instance
 * @return Thread return status
 */
ThreadReturn thread_mutex_init(ThreadMutex *mutex) {

    if (!mutex) {

        return THREAD_FAIL;
    }

    /* Set the owner to the none */
    mutex->owner_thread = NULL;

    /* Set the lock status to not acquired */
    mutex->lock_word = THREAD_LOCK_NOT_ACQUIRED;

    return THREAD_OK;
}

/**
 * @brief Acquires the mutex
 * @param[in/out] mutex Pointer to the mutex instance
 * @note The call is blocking and will return only if the lock is acquired
 * @return Thread return status
 */
ThreadReturn thread_mutex_lock(ThreadMutex *mutex) {

    Thread thread;
    int ret_val;

    /* Check for errors */
    if (!mutex) {

        return THREAD_FAIL;
    }

    /* Get the thread handle */
    thread = thread_self();

    /* Check if the current thread already owns the mutex */
    if (mutex->owner_thread == thread) {

        return THREAD_OK;
    }

    /* For eternity */
    while (1) {

        /* Atomically try to acquire the lock */
        if (atomic_cas(&mutex->lock_word,
                       THREAD_LOCK_NOT_ACQUIRED,
                       THREAD_LOCK_ACQUIRED)) {

            /* Set the owner to the current thread */
            mutex->owner_thread = thread;

            break;
        }

        /* Wait till the lock is not released by the current owner */
        ret_val = futex(&mutex->lock_word, FUTEX_WAIT, THREAD_LOCK_ACQUIRED);

        /* Check for errors */
        if ((ret_val == -1) && (errno != EAGAIN)) {

            return THREAD_FAIL;
        }
    }

    return THREAD_OK;
}

/**
 * @brief Releases the mutex
 * @param[in/out] mutex Pointer to the mutex instance
 * @return Thread return status
 */
ThreadReturn thread_mutex_unlock(ThreadMutex *mutex) {

    Thread thread;
    int ret_val;

    /* Check for errors */
    if (!mutex) {

        return THREAD_FAIL;
    }

    /* Get the thread handle */
    thread = thread_self();

    /* If the current thread is not the owner */
    if (mutex->owner_thread != thread) {

        /* Just return */
        return THREAD_OK;
    }

    /* Set the owner to no one */
    mutex->owner_thread = NULL;

    /* Release the lock atomically */
    if (atomic_cas(&mutex->lock_word,
                   THREAD_LOCK_ACQUIRED,
                   THREAD_LOCK_NOT_ACQUIRED)) {

        /* Wake up the waiting process */
        ret_val = futex(&mutex->lock_word, FUTEX_WAKE, NB_WAKEUP_PROCESSES);

        /* Check for errors */
        if ((ret_val == -1) && (errno != EAGAIN)) {

            return THREAD_FAIL;
        }
    }

    return THREAD_OK;
}

/**
 * @brief Initialize the spinlock
 * @param[in] spinlock Pointer to the spinlock instance
 * @return Thread return status
 */
ThreadReturn thread_spin_init(ThreadSpinLock *spinlock) {

    /* Check for errors */
    if (!spinlock) {

        return THREAD_FAIL;
    }

    /* Set the owner to the none */
    spinlock->owner_thread = NULL;

    /* Set the lock status to not acquired */
    spinlock->lock_word = THREAD_LOCK_NOT_ACQUIRED;

    return THREAD_OK;
}

/**
 * @brief Acquires the spinlock
 * @param[in/out] spinlock Pointer to the spinlock instance
 * @note The call is blocking and will return only if the lock is acquired
 * @return Thread return status
 */
ThreadReturn thread_spin_lock(ThreadSpinLock *spinlock) {

    Thread thread;

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

    /* While we dont get the lock */
    while (!atomic_cas(&spinlock->lock_word, THREAD_LOCK_NOT_ACQUIRED, THREAD_LOCK_ACQUIRED));

    /* Set the current thread as the owner */
    spinlock->owner_thread = thread;

    return THREAD_OK;
}

/**
 * @brief Releases the spinlock
 * @param[in/out] spinlock Pointer to the spinlock instance
 * @return Thread return status
 */
ThreadReturn thread_spin_unlock(ThreadSpinLock *spinlock) {

    Thread thread;

    /* Check for errors */
    if (!spinlock) {

        return THREAD_FAIL;
    }

    /* Get the thread handle */
    thread = thread_self();

    /* If the current thread does not own the lock */
    if (spinlock->owner_thread != thread) {

        return THREAD_OK;
    }

    /* Set the owner to none */
    spinlock->owner_thread = NULL;

    /* Release the lock */
    atomic_cas(&spinlock->lock_word, THREAD_LOCK_ACQUIRED, THREAD_LOCK_NOT_ACQUIRED);

    return THREAD_OK;
}
