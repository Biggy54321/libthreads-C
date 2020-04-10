#define _GNU_SOURCE
#include <sched.h>

#include "./mods/lock.h"
#include "./mods/utils.h"
#include "./mmrll.h"
#include "./thread.h"
#include "./thread_descr.h"

/* Next user thread identifier */
int nxt_utid;
/* Next user thread identifier lock */
Lock nxt_utid_lk;

/**
 * @brief Get next thread id
 *
 * Returns the thread id to be used for the next submitted user thread
 *
 * @return Integer id
 */
static int _get_nxt_utid(void) {

    int utid;

    /* Acquire the utid lock */
    lock_acquire(&nxt_utid_lk);

    /* Get the id */
    utid = nxt_utid++;

    /* Release the lock */
    lock_release(&nxt_utid_lk);

    return utid;
}

/**
 * @brief The actual start function of the thread
 *
 * This function launches the start function with the argument provided by
 * the application program
 */
static void _many_many_start(void) {

    Thread thread;

    /* Get the thread handle */
    thread = thread_self();

    /* Launch the thread start function */
    TD_LAUNCH(thread);

    /* Disable interrupt */
    TD_DISABLE_INTR(thread);

    /* Set the state as exited */
    TD_SET_STATE(thread, THREAD_STATE_EXITED);
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
        (!start)) {              /* If start function is not valid */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Allocate the thread descriptor */
    (*thread) = TD_ALLOC();
    /* Check for errors */
    if (!(*thread)) {

        /* Set the errno */
        thread_errno = EAGAIN;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Initialize the descriptor */
    TD_INIT(*thread, _get_nxt_utid(), start, arg);

    /* Initialize the start routine context */
    TD_INIT_CXT(*thread, _many_many_start);

    /* Acquire the many ready list lock */
    mmrll_lock();

    /* Add the current thread to the list  */
    mmrll_enqueue((*thread));

    /* Release the many ready list lock */
    mmrll_unlock();

    return THREAD_SUCCESS;
}

/**
 * @brief Joins with the target thread
 *
 * Waits for the target thread to complete its execution
 *
 * @param[in] thread Pointer to the thread handle
 * @param[out] ret Pointer to return value holder
 */
int thread_join(Thread thread, ptr_t *ret) {

    Thread curr_thread;

    /* Check for errors */
    if ((!thread) ||              /* If thread descriptor is not valid */
        (TD_IS_JOINED(thread))) { /* If target thread has already joined */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Get the current thread handle */
    curr_thread = thread_self();

    /* Check for deadlocks */
    if ((curr_thread == thread) ||                 /* Deadlock with itself */
        (TD_GET_JOINING(curr_thread) == thread)) { /* Deadlock with target */

        /* Set the errno */
        thread_errno = EDEADLK;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Acquire the member lock */
    TD_LOCK(thread);

    /* Check if the thread already has another joining thread */
    if (TD_HAS_JOINING(thread)) {

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Set the joining thread */
    TD_SET_JOINING(thread, curr_thread);

    /* Disable interrupts */
    TD_DISABLE_INTR(curr_thread);

    /* Update the current thread state */
    TD_SET_STATE(curr_thread, THREAD_STATE_WAIT_JOIN);

    /* Check if the target thread did not completed its execution */
    if (!TD_IS_OVER(thread)) {

        /* Release the member lock */
        TD_UNLOCK(thread);

        /* Return to the scheduler */
        TD_RET_CXT(curr_thread);

    } else {

        /* Release the member lock */
        TD_UNLOCK(thread);
    }

    /* Enable the interrupts */
    TD_ENABLE_INTR(curr_thread);

    /* If the return value is requested */
    if (ret) {

        /* Get the return value from the thread local storage */
        *ret = TD_GET_RET(thread);
    }

    /* Update the state of the target thread */
    TD_SET_STATE(thread, THREAD_STATE_JOINED);

    /* Free the memory and resources of the descriptor */
    TD_FREE(thread);

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

    /* Check for errors */
    if (TD_IS_EXITED(thread) || /* If thread has exited */
        TD_IS_JOINED(thread)) { /* If thread has joined */

        return;
    }

    /* Set the return value */
    TD_SET_RET(thread, ret);

    /* Disable interrupt */
    TD_DISABLE_INTR(thread);

    /* Set the thread state as exited */
    TD_SET_STATE(thread, THREAD_STATE_EXITED);

    /* Return to the scheduler */
    TD_EXIT_CXT(thread);
}

/**
 * @brief Return the calling thread handle
 *
 * Returns the handle of the calling thread, in order to perform operations on
 * itself if any
 */
Thread thread_self(void) {

    /* Return the value of FS register */
    return (Thread)get_fs();
}

/**
 * @brief Yields/returns the control to the scheduler
 */
int thread_yield(void) {

    Thread thread;

    /* Get the thread handle */
    thread = thread_self();

    /* Disable the interrupt */
    TD_DISABLE_INTR(thread);

    /* Yield to the scheduler */
    TD_RET_CXT(thread);

    /* Enable the interrupt */
    TD_ENABLE_INTR(thread);

    return 0;
}
