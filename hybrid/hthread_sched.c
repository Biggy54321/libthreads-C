#include <assert.h>

#include "./mods/utils.h"
#include "./mods/timer.h"
#include "./hthread_list.h"
#include "./hthread_sched.h"

/* Global scheduling status flag */
static int _do_scheduling;

/**
 * @brief Yield the control to the dispatcher from the user thread
 * @param[in] arg Not used
 */
void hthread_sched_yield(int arg) {

    HThread hthread;

    /* Get the address of the thread control block */
    hthread = BASE(get_fs());

    /* Check for errors */
    assert(hthread);

    /* Swap the context with the dispatcher */
    swapcontext(MANY_MANY(hthread)->curr_cxt, MANY_MANY(hthread)->ret_cxt);
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
    HThread hthread;
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
        hthread = hthread_list_get();

        /* Unlock the list */
        hthread_list_unlock();

        /* Set the FS register value to the TLS */
        set_fs((long)hthread);

        /* Start the timer */
        timer_start(&timer);

        /* Swap the context with the user thread */
        swapcontext(MANY_MANY(hthread)->ret_cxt, MANY_MANY(hthread)->curr_cxt);

        /* Stop the timer */
        timer_stop(&timer);

        /* Reset the FS register value to old value */
        set_fs(old_fs);

        /* Take action depending on the thread state */
        switch (hthread->state) {

            case HTHREAD_STATE_INIT:
            case HTHREAD_STATE_ACTIVE:

                /* Lock the list */
                hthread_list_lock();

                /* Add the current thread */
                hthread_list_add(hthread);

                /* Unlock the list */
                hthread_list_unlock();
                break;

            case HTHREAD_STATE_INACTIVE:

                /* Clear the wait state */
                hthread->wait = 0;
                break;

            default:
                break;
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
