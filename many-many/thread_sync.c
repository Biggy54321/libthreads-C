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
    (*spinlock) = alloc_mem(struct ThreadSpinLock);
    /* Check for errors */
    if (!(*spinlock)) {

        /* Set the errno */
        thread_errno = EAGAIN;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Set the owner to none */
    (*spinlock)->owner = NULL;

    /* Set the status to not acquired */
    lock_init(&(*spinlock)->lock);

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
    if ((*spinlock)->owner == thread) {

        return THREAD_SUCCESS;
    }

    /* Acquire the lock */
    lock_acquire(&(*spinlock)->lock);

    /* Set the owner to the current thread */
    (*spinlock)->owner = thread;

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
    if ((*spinlock)->owner != thread) {

        /* Set the errno */
        thread_errno = EACCES;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Set the owner to none */
    (*spinlock)->owner = NULL;

    /* Release the lock */
    lock_release(&(*spinlock)->lock);

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
    free(*spinlock);

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
    (*mutex) = alloc_mem(struct ThreadMutex);
    /* Check for errors */
    if (!(*mutex)) {

        /* Set the errno */
        thread_errno = EAGAIN;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Set the owner to none */
    (*mutex)->owner = NULL;

    /* Initialize the wait list */
    list_init(&(*mutex)->waitll);

    /* Initialize the member lock */
    lock_init(&(*mutex)->mem_lock);

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
    lock_acquire(&(*mutex)->mem_lock);

    /* If the current thread is the owner */
    if ((*mutex)->owner == thread) {

        /* Release the member lock */
        lock_release(&(*mutex)->mem_lock);

        return THREAD_SUCCESS;
    }

    /* If the lock is not owned */
    if (!(*mutex)->owner) {

        /* Set the owner as the current thread */
        (*mutex)->owner = thread;

        /* Release the member lock */
        lock_release(&(*mutex)->mem_lock);

        return THREAD_SUCCESS;
    }

    /* Disable interrupt */
    TD_DISABLE_INTR(thread);

    /* Update the state */
    TD_SET_STATE(thread, THREAD_STATE_WAIT_MUTEX);

    /* Set the wait for mutex */
    TD_SET_WAIT_MUTEX(thread, *mutex);

    /* Add the thread to the list */
    list_enqueue(&(*mutex)->waitll, thread, ll_mem);

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
    lock_acquire(&(*mutex)->mem_lock);

    /* If the owner of the mutex is not the current thread */
    if ((*mutex)->owner != thread) {

        /* Rel the list lock */
        lock_release(&(*mutex)->mem_lock);
        /* Set the errno */
        thread_errno = EACCES;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Check if list is not empty */
    if (!list_is_empty(&(*mutex)->waitll)) {

        /* Get the first waiting thread */
        wait_thread = list_dequeue(&(*mutex)->waitll, struct Thread, ll_mem);

        /* Set the owner as the wait thread */
        (*mutex)->owner = wait_thread;

        /* Acquire the many list lock */
        mmrll_lock();

        /* Add thread to the many many ready list */
        mmrll_enqueue(wait_thread);

        /* Acquire the many list lock */
        mmrll_unlock();
    } else {

        /* Set the owner to none */
        (*mutex)->owner = NULL;
    }

    /* Acquire the list lock */
    lock_release(&(*mutex)->mem_lock);

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
    free(*mutex);

    return THREAD_SUCCESS;
}
