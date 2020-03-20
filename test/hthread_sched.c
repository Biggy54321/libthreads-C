#include <assert.h>

#include "./lib/utils.h"
#include "./lib/timer.h"
#include "./hthread_list.h"
#include "./hthread_sched.h"

/* Global scheduling status flag */
static int _do_scheduling;

/**
 * @brief Yield the control to the dispatcher from the user thread
 * @param[in] arg Not used
 */
void hthread_sched_yield(int arg) {

    struct _HThreadManyMany *hthread;

    /* Get the address of the thread control block */
    hthread = (struct _HThreadManyMany *)get_fs();

    /* Check for errors */
    assert(hthread);

    /* Swap the context with the dispatcher */
    swapcontext(hthread->curr_cxt, hthread->ret_cxt);
}

/**
 * @brief Dispatch a user thread onto a kernel thread
 *
 * Continuously selects a thread from the global list of threads and schedules
 * it on the kernel thread on which the function is itself running
 *
 * @param[in] arg Not used
 * @return Integer (not used)
 */
int hthread_sched_dispatch(void *arg) {

    Timer timer;
    struct _HThreadManyMany *hthread;
    long old_fs;

    /* Initialize the one shot timer event */
    timer_set(&timer, hthread_sched_yield, TIME_SLICE_ms);

    /* Get the current FS register value */
    old_fs = get_fs();

    /* While scheduling is allowed */
    while (_do_scheduling) {

        /* Lock the thread list */
        hthread_list_lock();

        /* If the list is empty */
        if (hthread_list_is_empty()) {

            /* Unlock and continue */
            hthread_list_unlock();
            continue;
        }

        /* Get the thread to be scheduled */
        hthread = (struct _HThreadManyMany *)hthread_list_get();

        /* Unlock the list */
        hthread_list_unlock();

        /* Set the FS register value to the TLS */
        set_fs((long)hthread);

        /* Start the timer */
        timer_start(&timer);

        /* Swap the context with the user thread */
        swapcontext(hthread->ret_cxt, hthread->curr_cxt);

        /* Stop the timer */
        timer_stop(&timer);

        /* Reset the FS register value to old value */
        set_fs(old_fs);

        /* If the thread is active */
        if (hthread->state == HTHREAD_STATE_ACTIVE) {

            /* Lock the list */
            hthread_list_lock();

            /* Add the current thread */
            hthread_list_add(hthread);

            /* Unlock the list */
            hthread_list_unlock();
        }
    }
}

/**
 * @brief Enable the scheduling operation
 * @note This should be done before any kernel threads are created.
 */
void hthread_sched_start(void) {

    /* Set the flag */
    _do_scheduling = 1;
}

/**
 * @brief Disable the scheduling operation
 * @note This should be done to stop the kernel threads from scheduling
 */
void hthread_sched_stop(void) {

    /* Clear the flag */
    _do_scheduling = 0;
}
