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
    td_launch(thread);

    /* Disable interrupt */
    td_disable_intr(thread);

    /* Set the state as exited */
    td_set_state(thread, THREAD_STATE_EXITED);
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
    (*thread) = td_alloc();
    /* Check for errors */
    if (!(*thread)) {

        /* Set the errno */
        thread_errno = EAGAIN;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Initialize the descriptor */
    td_init(*thread, _get_nxt_utid(), start, arg);

    /* Initialize the start routine context */
    td_init_cxt(*thread, _many_many_start);

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
        (td_is_joined(thread))) { /* If target thread has already joined */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Get the current thread handle */
    curr_thread = thread_self();

    /* Check for deadlocks */
    if ((curr_thread == thread) ||                 /* Deadlock with itself */
        (td_get_joining(curr_thread) == thread)) { /* Deadlock with target */

        /* Set the errno */
        thread_errno = EDEADLK;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Acquire the member lock */
    td_lock(thread);

    /* Check if the thread already has another joining thread */
    if (td_has_joining(thread)) {

        /* Release the member lock */
        td_unlock(thread);
        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Disable interrupts */
    td_disable_intr(curr_thread);

    /* Set the joining thread */
    td_set_joining(thread, curr_thread);

    /* Set the thread, the calling thread is waiting for */
    td_set_wait_thread(curr_thread, thread);

    /* Update the current thread state */
    td_set_state(curr_thread, THREAD_STATE_WAIT_JOIN);

    /* Check if the target thread did not completed its execution */
    if (!td_is_over(thread)) {

        /* Return to the scheduler */
        td_ret_cxt(curr_thread);

    } else {

        /* Release the member lock */
        td_unlock(thread);
    }

    /* Change the state of the calling thread to running */
    td_set_state(curr_thread, THREAD_STATE_RUNNING);

    /* Clear the wait for thread */
    td_set_wait_thread(curr_thread, NULL);

    /* Enable the interrupts */
    td_enable_intr(curr_thread);

    /* If the return value is requested */
    if (ret) {

        /* Get the return value from the thread local storage */
        *ret = td_get_ret(thread);
    }

    /* Update the state of the target thread */
    td_set_state(thread, THREAD_STATE_JOINED);

    /* Free the memory and resources of the descriptor */
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

    /* Check for errors */
    if (td_is_exited(thread) || /* If thread has exited */
        td_is_joined(thread)) { /* If thread has joined */

        return;
    }

    /* Set the return value */
    td_set_ret(thread, ret);

    /* Disable interrupt */
    td_disable_intr(thread);

    /* Set the thread state as exited */
    td_set_state(thread, THREAD_STATE_EXITED);

    /* Return to the scheduler */
    td_exit_cxt(thread);
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
    td_disable_intr(thread);

    /* Yield to the scheduler */
    td_ret_cxt(thread);

    /* Enable the interrupt */
    td_enable_intr(thread);

    return THREAD_SUCCESS;
}

/**
 * @brief Checks if the two thread descriptors are same
 * @param[in] thread1 First thread handle
 * @param[in] thread2 Second thread handle
 * @return 0 if not equal
 * @return 1 if equal
 */
int thread_equal(Thread thread1, Thread thread2) {

    /* Check if the two thread handles store the same value, which will
     * be the address of the thread descriptor they are pointing to */
    return (thread1 == thread2);
}
