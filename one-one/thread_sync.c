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

        /* Update the thread state */
        thread->thread_state = THREAD_STATE_WAIT_MUTEX;

        /* Wait till the lock is not released by the current owner */
        ret_val = futex(&mutex->lock_word, FUTEX_WAIT, THREAD_LOCK_ACQUIRED);

        /* Check for errors */
        if ((ret_val == -1) && (errno != EAGAIN)) {

            return THREAD_FAIL;
        }

        /* Update the thread state */
        thread->thread_state = THREAD_STATE_RUNNING;
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

    /* Update the thread state */
    thread->thread_state = THREAD_STATE_WAIT_SPINLOCK;

    /* While we dont get the lock */
    while (!atomic_cas(&spinlock->lock_word,
                       THREAD_LOCK_NOT_ACQUIRED,
                       THREAD_LOCK_ACQUIRED));

    /* Update the thread state */
    thread->thread_state = THREAD_STATE_RUNNING;

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
    atomic_cas(&spinlock->lock_word,
               THREAD_LOCK_ACQUIRED,
               THREAD_LOCK_NOT_ACQUIRED);

    return THREAD_OK;
}

/**
 * @brief Initialize the condition variable
 * @param[in] cond Pointer to the condition variable instance
 */
ThreadReturn thread_cond_init(ThreadCond *cond) {

    /* Check for errors */
    if (!cond) {

        return THREAD_FAIL;
    }

    /* Set the number of waiting threads */
    cond->nb_threads = 0;

    /* Set the mutex linked with the condition variable */
    cond->mutex = NULL;

    /* Set the wait word to zero */
    cond->zero = 0;

    return THREAD_OK;
}

/**
 * @brief Wait on a condition variable
 * @param[in] cond Pointer to the condition variable instance
 * @param[in] mutex Pointer to the mutex instance
 */
ThreadReturn thread_cond_wait(ThreadCond *cond, ThreadMutex *mutex) {

    Thread thread;

    /* Check for errors */
    if (!cond) {

        return THREAD_FAIL;
    }
    if (!mutex) {

        return THREAD_FAIL;
    }

    /* Check if mutex is locked */
    if (mutex->lock_word == THREAD_LOCK_NOT_ACQUIRED) {

        return THREAD_FAIL;
    }

    /* Get the thread handle */
    thread = thread_self();

    /* Check if the current thread is the owner of the lock */
    if (mutex->owner_thread != thread) {

        return THREAD_FAIL;
    }

    /* If the number of waiting threads is zero */
    if (!cond->nb_threads) {

        /* Link the mutex and the condition variable */
        cond->mutex = mutex;
    } else {

        /* If the already linked mutex does not match the passed mutex */
        if (cond->mutex != mutex) {

            return THREAD_FAIL;
        }
    }

    /* Update the number of waiting threads */
    cond->nb_threads++;

    /* Release the mutex */
    thread_mutex_unlock(mutex);

    /* Update the state */
    thread->thread_state = THREAD_STATE_WAIT_COND;

    /* Wait on the wait word */
    futex(&cond->zero, FUTEX_WAIT, 0);

    /* Update the state */
    thread->thread_state = THREAD_STATE_RUNNING;

    /* Acquire the mutex */
    thread_mutex_lock(mutex);

    /* Update the number of waiting threads */
    cond->nb_threads--;

    /* If the no waiting threads then remove the mutex link */
    if (!cond->nb_threads) {

        cond->mutex = NULL;
    }

    return THREAD_OK;
}

/**
 * @brief Signal a single waiting thread to wake up which is already waiting
 *        on the given condition variable
 * @param[in] cond Pointer to the condition variable instance
 */
ThreadReturn thread_cond_signal(ThreadCond *cond) {

    /* Check for errors */
    if (!cond) {

        return THREAD_FAIL;
    }

    /* If the number of threads waiting are zero */
    if (!cond->nb_threads) {

        return THREAD_OK;
    }

    /* Wake up only one thread */
    futex(&cond->zero, FUTEX_WAKE, 1);

    return THREAD_OK;
}

/**
 * @brief Signal a all the waiting thread to wake up which is already waiting
 *        on the given condition variable
 * @param[in] cond Pointer to the condition variable instance
 */
ThreadReturn thread_cond_broadcast(ThreadCond *cond) {

    /* Check for errors */
    if (!cond) {

        return THREAD_FAIL;
    }

    /* If the number of threads waiting are zero */
    if (!cond->nb_threads) {

        return THREAD_OK;
    }

    /* Wake up only one thread */
    futex(&cond->zero, FUTEX_WAKE, cond->nb_threads);

    return THREAD_OK;
}
