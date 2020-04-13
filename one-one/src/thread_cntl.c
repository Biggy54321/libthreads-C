#include "./mods/utils.h"
#include "./thread.h"
#include "./thread_descr.h"

/**
 * @brief The thread start function which initiates the actual start
 *        function
 * @param[in] arg Not used
 * @return Integer
 */
static int _one_one_start(void *arg) {

    Thread thread;

    /* Get the thread handle */
    thread = thread_self();

    /* Launch the thread */
    td_launch(thread);

    /* Set the state as exited */
    td_set_state(thread, THREAD_STATE_EXITED);

    return 0;
}

/**
 * @brief Create a thread
 *
 * Creates a many-many thread by allocating it the requried resources and
 * adding it to the required book-keeping data structures
 *
 * @param[out] thread Pointer to the thread handle
 * @param[in] start Start routine
 * @param[in] arg Argument to the start routine
 */
int thread_create(Thread *thread, thread_start_t start, ptr_t arg) {

    /* Check for errors */
    if ((!thread) ||            /* If thread descriptor is not valid */
        (!start)) {             /* If start function is not valid */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Allocate the thread descriptor */
    *thread = td_alloc();

    /* Check for errors */
    if (!(*thread)) {

        /* Set the errno */
        thread_errno = EAGAIN;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Initialize the thread descriptor */
    td_init(*thread, start, arg);

    /* Create the thread */
    td_create(*thread, _one_one_start);

    /* Check for errors */
    if (td_get_ktid(*thread) == -1) {

        /* Set the errno */
        thread_errno = EAGAIN;
        /* Return failure */
        return THREAD_FAIL;
    }

    return THREAD_SUCCESS;
}

/**
 * @brief Waits for the specified target thread to stop
 * @param[in] thread Thread handle
 * @param[out] return_value Pointer to the return value
 */
int thread_join(Thread thread, ptr_t *ret) {

    Thread curr_thread;

    /* Check for errors */
    if ((!thread) ||
        td_is_joined(thread)) {

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Get the thread handle */
    curr_thread = thread_self();

    /* Check for deadlock with itself */
    if ((thread == curr_thread) ||
        (td_get_join_thread(curr_thread) == thread)) {

        /* Set the errno */
        thread_errno = EDEADLK;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Acquire the member lock */
    td_lock(thread);

    /* Check if another thread is already waiting */
    if (td_has_join_thread(thread)) {

        /* Release the member lock */
        td_unlock(thread);
        /* Set the error number */
        thread_errno = EINVAL;
        return THREAD_FAIL;
    }

    /* Set the current thread as the joining thread */
    td_set_join_thread(thread, curr_thread);

    if (!td_is_over(thread)) {

        /* Release the member lock */
        td_unlock(thread);

        /* Update the state of the current thread */
        td_set_state(curr_thread, THREAD_STATE_WAIT_JOIN);

        /* Wait */
        futex(&thread->wait, FUTEX_WAIT, td_get_ktid(thread));

        /* Update the state of the current thread */
        td_set_state(curr_thread, THREAD_STATE_RUNNING);
    } else {

        /* Release the member lock */
        td_unlock(thread);
    }

    /* Update the target thread state */
    td_set_state(thread, THREAD_STATE_JOINED);

    /* If the return value is requested */
    if (ret) {

        /* Get the return value from the thread descriptor */
        *ret = td_get_ret(thread);
    }

    /* Free the thread descriptor */
    td_free(thread);

    return THREAD_SUCCESS;
}

/**
 * @brief Exit from the thread
 *
 * Stops the execution of the thread, and gives a return status
 *
 * @param[in] ret Return status
 */
void thread_exit(ptr_t ret) {

    Thread thread;

    /* Get the thread handle */
    thread = thread_self();

    /* Set the return value */
    td_set_ret(thread, ret);

    /* Update the thread state */
    td_set_state(thread, THREAD_STATE_EXITED);

    /* Exit (using the system call rather than the glibc wrapper) */
    sys_exit(0);

}

/**
 * @brief Yields the control from the calling thread. I.e. it gives up the
 *        CPU
 */
int thread_yield(void) {

    /* Yield to the scheduler */
    return sched_yield();
}

/**
 * @brief Returns the thread handle of the current thread
 * @return Thread handle
 * @note The thread handle will be valid only if the thread is created using
 *       thread_create()
 */
Thread thread_self(void) {

    /* Read the FS register value */
    return (Thread)get_fs();
}
