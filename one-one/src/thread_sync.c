#include "./mods/utils.h"
#include "./thread.h"
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
    (*spinlock) = spin_alloc();

    /* Check for errors */
    if (!(*spinlock)) {

        /* Set the errno */
        thread_errno = EAGAIN;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Initialize the spinlock */
    spin_init(*spinlock);

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
    if (!(spinlock) ||           /* Pointer to spinlock is valid */
        !(*spinlock)) {          /* The argument points to a structure */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Get the thread handle */
    thread = thread_self();

    /* If the current thread is the owner */
    if (spin_get_owner(*spinlock) == thread) {

        return THREAD_SUCCESS;
    }

    /* While the lock is not acquired */
    while (!spin_acq_lock(*spinlock));

    /* Set the owner to the current thread */
    spin_set_owner(*spinlock, thread);

    return THREAD_SUCCESS;
}

/**
 * @brief Tries to acquire the spinlock
 *
 * Acquires the spinlock if it is not acquired by any other thread. However if
 * the thread is already acquired then the call does not block.
 *
 * @param[in] spinlock Pointer to the spinlock instance
 */
int thread_spin_trylock(ThreadSpinLock *spinlock) {

    Thread thread;

    /* Check for errors */
    if (!(spinlock) ||           /* Pointer to spinlock is valid */
        !(*spinlock)) {          /* The argument points to a structure */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Get the thread handle */
    thread = thread_self();

    /* If the current thread is the owner */
    if (spin_get_owner(*spinlock) == thread) {

        return THREAD_SUCCESS;
    }

    /* Acquire the lock */
    if (!spin_acq_lock(*spinlock)) {

        /* Set the errno */
        thread_errno = EBUSY;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Set the owner to the current thread */
    spin_set_owner(*spinlock, thread);

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
    if (!(spinlock) ||           /* Pointer to spinlock is valid */
        !(*spinlock)) {          /* The argument points to a structure */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Get the thread handle */
    thread = thread_self();

    /* If the owner of the spinlock is not the current thread */
    if (spin_get_owner(*spinlock) != thread) {

        /* Set the errno */
        thread_errno = EACCES;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Set the owner to none */
    spin_set_owner(*spinlock, NULL);

    /* Release the lock */
    spin_rel_lock(*spinlock);

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
    if (!(spinlock) ||           /* Pointer to spinlock is valid */
        !(*spinlock)) {          /* The argument points to a structure */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Free the allocated memory */
    spin_free(*spinlock);

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
    (*mutex) = mut_alloc();

    /* Check for errors */
    if (!(*mutex)) {

        /* Set the errno */
        thread_errno = EAGAIN;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Initialize the mutex */
    mut_init(*mutex);

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
    if (!(mutex) ||           /* Pointer to mutex is valid */
        !(*mutex)) {          /* The argument points to a structure */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Get the thread handle */
    thread = thread_self();

    /* If the current thread is the owner */
    if (mut_get_owner(*mutex) == thread) {

        return THREAD_SUCCESS;
    }

    /* While the lock is not acquired */
    while (!mut_acq_lock(*mutex)) {

        /* Update the state */
        td_set_state(thread, THREAD_STATE_WAIT_MUTEX);

        /* Wait */
        mut_wait(*mutex);

        /* Update the state */
        td_set_state(thread, THREAD_STATE_RUNNING);
    }

    /* Set the owner as the current thread */
    mut_set_owner(*mutex, thread);

    return THREAD_SUCCESS;
}

/**
 * @brief Tries to acquire the mutex
 *
 * Acquires the mutex if it is not acquired by any other thread. However if
 * the thread is already acquired then the call does not block.
 *
 * @param[in] mutex Pointer to the mutex instance
 */
int thread_mutex_trylock(ThreadMutex *mutex) {

    Thread thread;

    /* Check for errors */
    if (!(mutex) ||           /* Pointer to mutex is valid */
        !(*mutex)) {          /* The argument points to a structure */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Get the thread handle */
    thread = thread_self();

    /* If the current thread is the owner */
    if (mut_get_owner(*mutex) == thread) {

        return THREAD_SUCCESS;
    }

    /* If lock is not acquired */
    if (!mut_acq_lock(*mutex)) {

        /* Set the errno */
        thread_errno = EBUSY;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Set the owner as the current thread */
    mut_set_owner(*mutex, thread);

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

    /* Check for errors */
    if (!(mutex) ||           /* Pointer to mutex is valid */
        !(*mutex)) {          /* The argument points to a structure */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Get the thread handle */
    thread = thread_self();

    /* If the current thread is not the owner */
    if (mut_get_owner(*mutex) != thread) {

        /* Set the errno */
        thread_errno = EACCES;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Set the owner to null */
    mut_set_owner(*mutex, NULL);

    /* Release the lock */
    mut_rel_lock(*mutex);

    /* Wake up one waiting processes */
    mut_wake(*mutex);

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
    if (!(mutex) ||           /* Pointer to mutex is valid */
        !(*mutex)) {          /* The argument points to a structure */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Free the mutex object */
    mut_free(*mutex);

    return THREAD_SUCCESS;
}
