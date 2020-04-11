#include <stdlib.h>
#include <stddef.h>

#include "./mods/utils.h"
#include "./mmrll.h"
#include "./thread_descr.h"
#include "./thread_sync.h"

/**
 * @brief Initializes the spinlock
 *
 * Allocates the required memory for the spinlock object and initializes it
 * to the base value
 *
 * @param[in] spinlock Pointer to the spinlock instance
 */
int thread_spin_init(ThreadSpinLock *spinlock) {

    /* Check for errors */
    if (!spinlock) {            /* If pointer to spinlock is invalid */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Allocate the memory */
    (*spinlock) = SPIN_ALLOC();

    /* Check for errors */
    if (!(*spinlock)) {

        /* Set the errno */
        thread_errno = EAGAIN;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Initialize the spinlock */
    SPIN_INIT(*spinlock);

    return THREAD_SUCCESS;
}

/**
 * @brief Acquires the spinlock
 *
 * Acquires the spinlock. The calling thread will busy wait till the lock is
 * not acquired, i.e. the thread will not be blocked and will consume CPU
 * while waiting
 *
 * @param[in] spinlock Pointer to the spinlock instance
 */
int thread_spin_lock(ThreadSpinLock *spinlock) {

    Thread thread;

    /* Check for errors */
    if ((spinlock) ||           /* Pointer to spinlock is valid */
        (*spinlock)) {          /* The argument points to a structure */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Get the thread handle */
    thread = thread_self();

    /* If the current thread is the owner */
    if (SPIN_GET_OWNER(*spinlock) == thread) {

        return THREAD_SUCCESS;
    }

    /* Acquire the lock */
    SPIN_ACQ_LOCK(*spinlock);

    /* Set the owner to the current thread */
    SPIN_SET_OWNER(*spinlock, thread);

    return THREAD_SUCCESS;
}

/**
 * @brief Releases the spinlock
 *
 * Releases the spinlock and sets the owner of the lock to no one.
 *
 * @param[in] spinlock Pointer to the spinlock instance
 */
int thread_spin_unlock(ThreadSpinLock *spinlock) {

    Thread thread;

    /* Check for errors */
    if ((spinlock) ||           /* Pointer to spinlock is valid */
        (*spinlock)) {          /* The argument points to a structure */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Get the thread handle */
    thread = thread_self();

    /* If the owner of the spinlock is not the current thread */
    if (SPIN_GET_OWNER(*spinlock) != thread) {

        /* Set the errno */
        thread_errno = EACCES;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Set the owner to none */
    SPIN_SET_OWNER(*spinlock, NULL);

    /* Release the lock */
    SPIN_REL_LOCK(*spinlock);

    return THREAD_SUCCESS;
}

/**
 * @brief Destroy the spinlock
 *
 * Free the allocated memory for the spinlock object
 *
 * @param[in] spinlock Pointer to the spinlock instance
 */
int thread_spin_destroy(ThreadSpinLock *spinlock) {

    /* Check for errors */
    if ((spinlock) ||           /* Pointer to spinlock is valid */
        (*spinlock)) {          /* The argument points to a structure */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Free the allocated memory */
    SPIN_FREE(*spinlock);

    return THREAD_SUCCESS;
}

/**
 * @brief Initializes the mutex
 *
 * Allocates memory for the mutex object and sets the members to the base
 * values
 *
 * @param[in] mutex Pointer to the mutex instance
 */
int thread_mutex_init(ThreadMutex *mutex) {

    /* Check for errors */
    if (!mutex) {            /* If pointer to mutex is invalid */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Allocate the memory */
    (*mutex) = MUT_ALLOC();

    /* Check for errors */
    if (!(*mutex)) {

        /* Set the errno */
        thread_errno = EAGAIN;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Initialize the mutex */
    MUT_INIT(*mutex);

    return THREAD_SUCCESS;
}

/**
 * @brief Acquires the mutex
 *
 * Acquires the mutex and sets the owner of the lock to the calling thread.
 * The function does not return unless the lock is acquired. However a waiting
 * thread will not consume CPU
 *
 * @param[in] mutex Pointer to the mutex instance
 */
int thread_mutex_lock(ThreadMutex *mutex) {

    Thread thread;

    /* Check for errors */
    if ((mutex) ||           /* Pointer to mutex is valid */
        (*mutex)) {          /* The argument points to a structure */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Get the thread handle */
    thread = thread_self();

    /* Acquire the member lock */
    MUT_LOCK(*mutex);

    /* If the current thread is the owner */
    if (MUT_GET_OWNER(*mutex) == thread) {

        /* Release the member lock */
        lock_release(&(*mutex)->mem_lock);

        return THREAD_SUCCESS;
    }

    /* If the lock is not owned */
    if (!MUT_HAS_OWNER(*mutex)) {

        /* Set the owner as the current thread */
        MUT_SET_OWNER(*mutex, thread);

        /* Release the member lock */
        MUT_UNLOCK(*mutex);

        return THREAD_SUCCESS;
    }

    /* Disable interrupt */
    TD_DISABLE_INTR(thread);

    /* Update the state */
    TD_SET_STATE(thread, THREAD_STATE_WAIT_MUTEX);

    /* Set the wait for mutex */
    TD_SET_WAIT_MUTEX(thread, *mutex);

    /* Add the thread to the list */
    MUT_ADD_WAIT_THREAD(*mutex, thread);

    /* Return to the scheduler */
    TD_RET_CXT(thread);

    /* Clear the wait for mutex */
    TD_SET_WAIT_MUTEX(thread, NULL);

    /* Update the state */
    TD_SET_STATE(thread, THREAD_STATE_RUNNING);

    /* Enabe the interrupt */
    TD_ENABLE_INTR(thread);

    return THREAD_SUCCESS;
}

/**
 * @brief Release the mutex lock
 *
 * Releases the mutex lock, and provides the access to the lock to another
 * thread which has been waiting for the mutex previously
 *
 * @param[in] mutex Pointer to the mutex instance
 */
int thread_mutex_unlock(ThreadMutex *mutex) {

    Thread thread;
    Thread wait_thread;

    /* Check for errors */
    if ((mutex) ||           /* Pointer to mutex is valid */
        (*mutex)) {          /* The argument points to a structure */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Get the thread handle */
    thread = thread_self();

    /* Acquire the list lock */
    MUT_LOCK(*mutex);

    /* If the owner of the mutex is not the current thread */
    if (MUT_GET_OWNER(*mutex) != thread) {

        /* Rel the list lock */
        MUT_UNLOCK(*mutex);
        /* Set the errno */
        thread_errno = EACCES;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* If there are threads waiting */
    if (MUT_HAS_WAIT_THREAD(*mutex)) {

        /* Get the first waiting thread */
        wait_thread = MUT_GET_WAIT_THREAD(*mutex);

        /* Set the owner as the wait thread */
        MUT_SET_OWNER(*mutex, wait_thread);

        /* Acquire the many list lock */
        mmrll_lock();

        /* Add thread to the many many ready list */
        mmrll_enqueue(wait_thread);

        /* Acquire the many list lock */
        mmrll_unlock();
    } else {

        /* Set the owner to none */
        MUT_SET_OWNER(*mutex, NULL);
    }

    /* Acquire the list lock */
    MUT_UNLOCK(*mutex);

    return THREAD_SUCCESS;
}

/**
 * @brief Destroy the mutex
 *
 * Free the allocated memory for the mutex object
 *
 * @param[in] mutex Pointer to the mutex instance
 */
int thread_mutex_destroy(ThreadMutex *mutex) {

    /* Check for errors */
    if ((mutex) ||           /* Pointer to mutex is valid */
        (*mutex)) {          /* The argument points to a structure */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Free the mutex object */
    MUT_FREE(*mutex);

    return THREAD_SUCCESS;
}
