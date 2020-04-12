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

    /* Acquire the member lock */
    td_lock(thread);

    /* If the current thread has a joining */
    if (td_has_join_thread(thread)) {

        /* If the joining thread is of type many many */
        if (td_is_many_many(td_get_join_thread(thread))) {

            /* Release the member lock */
            td_unlock(thread);

            /* Lock the ready list */
            mmrll_lock();

            /* Add the current thread to the ready list */
            mmrll_enqueue(td_get_join_thread(thread));

            /* Unlock the ready list */
            mmrll_unlock();
        }
    } else {

        /* Release the member lock */
        td_unlock(thread);
    }

    return 0;
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

    /* Launch the thread */
    td_launch(thread);

    /* Disable interrupts */
    td_mm_disable_intr(thread);

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
 * @param[in] type Type of the thread
 */
void thread_create(Thread *thread, thread_start_t start, ptr_t arg, int type) {

    /* Check for errors */
    assert(thread);
    assert(start);
    assert((type == THREAD_TYPE_ONE_ONE) || (type == THREAD_TYPE_MANY_MANY));

    /* If thread type is one one */
    if (type == THREAD_TYPE_ONE_ONE) {

        /* Allocate the thread */
        *thread = td_oo_alloc();

        /* Initialize the thread */
        td_oo_init(*thread, _get_nxt_utid(), start, arg);

        /* Create the thread */
        td_oo_create(*thread, _one_one_start);

    } else {

        /* Allocate the thread */
        *thread = td_mm_alloc();

        /* Initialize the thread */
        td_mm_init(*thread, _get_nxt_utid(), start, arg);

        /* Create the thread */
        td_mm_create(*thread, _many_many_start);

        /* Acquire the many ready list lock */
        mmrll_lock();
        /* Add the current thread to the list  */
        mmrll_enqueue(*thread);
        /* Release the many ready list lock */
        mmrll_unlock();
    }
}

/**
 * @brief Joins with the target thread
 *
 * Waits for the target thread to complete its execution
 *
 * @param[in] thread Pointer to the thread handle
 * @param[out] ret Pointer to return value holder
 */
void thread_join(Thread thread, ptr_t *ret) {

    int wait_val;
    Thread curr_thread;

    /* Check for errors */
    assert(thread);
    assert(!td_is_joined(thread));

    /* Get the current thread handle */
    curr_thread = thread_self();

    /* Check for deadlock with itself */
    assert(curr_thread != thread);

    /* Check for deadlock with target thread */
    assert(td_get_join_thread(curr_thread) != thread);

    /* Acquire the member lock */
    td_lock(thread);

    /* Check if the thread already has another joining thread */
    assert(!td_has_join_thread(thread));

    /* If the current thread type is many many */
    if (td_is_many_many(curr_thread)) {

        /* Disable the interrupts */
        td_mm_disable_intr(curr_thread);
    }

    /* Set the joining thread */
    td_set_join(thread, curr_thread);

    /* Set the state as waiting for current thread */
    td_set_state(curr_thread, THREAD_STATE_WAIT_JOIN);

    /* If target thread is not over */
    if (!td_is_over(thread)) {

        /* If the current thread is one one */
        if (td_is_one_one(curr_thread)) {

            /* Release the member lock */
            td_unlock(thread);

            /* If the target thread id one one */
            if (td_is_one_one(thread)) {

                wait_val = td_oo_get_ktid(thread);
            } else {

                wait_val = 1;
            }

            /* Wait */
            futex(&thread->wait, FUTEX_WAIT, wait_val);

        } else {

            /* Return to scheduler */
            td_mm_ret_cxt(curr_thread);

            /* If the target thread type is one one */
            if (td_is_one_one(thread)) {

                /* Wait till the wait word becomes zero */
                while (thread->wait);
            }
        }

    } else {

        /* Release the member lock */
        td_unlock(thread);
    }

    /* Change the state of the calling thread to running */
    td_set_state(curr_thread, THREAD_STATE_RUNNING);

    /* Clear the wait for thread */
    td_clear_wait_join_thread(curr_thread);

    /* If the current thread type is many many */
    if (td_is_many_many(curr_thread)) {

        /* Enable the interrupts */
        td_mm_enable_intr(curr_thread);
    }

    /* If the return value is requested */
    if (ret) {

        /* Get the return value from the thread local storage */
        *ret = td_get_ret(thread);
    }

    /* Update the state of the target thread */
    td_set_state(thread, THREAD_STATE_JOINED);

    /* Free the target thread descriptor */
    if (td_is_one_one(thread)) {

        td_oo_free(thread);
    } else {

        td_mm_free(thread);
    }
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

    /* Check if thread is not dead or exited */
    assert(thread->state != THREAD_STATE_EXITED);
    assert(thread->state != THREAD_STATE_JOINED);

    /* Set the return value */
    td_set_ret(thread, ret);

    /* If the type is many many */
    if (td_is_many_many(thread)) {

        /* Disable interrupts */
        td_mm_disable_intr(thread);
    }

    /* Set the thread state as exited */
    td_set_state(thread, THREAD_STATE_EXITED);

    /* Exit depending on the thread type */
    if (td_is_one_one(thread)) {

        /* Acquire the member lock */
        td_lock(thread);

        /* If the current thread has a joining */
        if (td_has_join_thread(thread)) {

            /* If the joining thread is of type many many */
            if (td_is_many_many(td_get_join_thread(thread))) {

                /* Release the member lock */
                td_unlock(thread);

                /* Lock the ready list */
                mmrll_lock();

                /* Add the current thread to the ready list */
                mmrll_enqueue(td_get_join_thread(thread));

                /* Unlock the ready list */
                mmrll_unlock();
            }
        } else {

            /* Release the member lock */
            td_unlock(thread);
        }

        sys_exit(0);
    } else {

        td_mm_exit_cxt(thread);
    }
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
void thread_yield(void) {

    Thread thread;

    /* Get the thread handle */
    thread = thread_self();

    /* If the thread type is one one */
    if (td_is_one_one(thread)) {

        sched_yield();
    } else {

        /* Disable the interrupt */
        td_mm_disable_intr(thread);

        /* Yield to the scheduler */
        td_mm_ret_cxt(thread);

        /* Enable the interrupt */
        td_mm_enable_intr(thread);
    }
}
