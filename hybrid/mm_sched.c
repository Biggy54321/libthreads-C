#define _GNU_SOURCE
#include <sched.h>

#include "./mods/utils.h"
#include "./mods/list.h"
#include "./mods/lock.h"
#include "./mods/stack.h"
#include "./mods/sig.h"
#include "./mods/timer.h"
#include "./mm_rdy_list.h"
#include "./mm_sched.h"
#include "./hthread_priv.h"
#include "./hthread_cntl.h"
#include "./hthread_sig.h"

/* Clone flags for the kernel thread of the scheduler */
#define _MM_SCHED_CLONE_FLAGS                   \
    (CLONE_VM | CLONE_FS | CLONE_FILES |        \
     CLONE_SIGHAND | CLONE_THREAD |             \
     CLONE_SYSVSEM | CLONE_PARENT_SETTID |      \
     CLONE_CHILD_CLEARTID)

/* Scheduler list */
static List mm_sched_list = LIST_INITIALIZER;
/* Scheduling status */
static int mm_sched_enabled = 0;

/**
 * @brief Yield the control to the dispatcher from the user thread
 * @param[in] arg Not used
 */
static void _mm_sched_yield(int arg) {

    HThread hthread;

    /* Get the thread handle */
    hthread = hthread_self();

    /* Swap the context with the dispatcher */
    swapcontext(MANY_TLS(hthread)->curr_cxt, MANY_TLS(hthread)->ret_cxt);
}

/**
 * @brief Dispatch a user thread
 *
 * Continuously selects a thread from the global list of threads and schedules
 * it on the kernel thread on which the function is itself running
 *
 * @param[in] arg Not used
 * @return Integer (not used)
 */
static int _mm_sched_dispatch(void *arg) {

    HThread hthread;
    Timer timer;
    void *old_fs;
    Signal *sig;
    struct sigaction act;

    /* Initialize the action for the timer event */
    act.sa_handler = _mm_sched_yield;
    act.sa_flags = 0;
    sigfillset(&act.sa_mask);

    /* Block all the signals */
    sig_block_all();

    /* Set the timer for the above action */
    timer_set(&timer, act, MM_SCHED_TIME_SLICE);

    /* Get the current FS register value */
    old_fs = get_fs();

    /* While the scheduling is enabled */
    while (mm_sched_enabled) {

        /* Lock the ready list */
        mm_rdy_list_lock();

        /* If the ready list is empty */
        if (mm_rdy_list_is_empty()) {

            /* Unlock the list and continue */
            mm_rdy_list_unlock();
            continue;
        }

        /* Get a thread from the list */
        hthread = mm_rdy_list_get();

        /* Unlock the ready list */
        mm_rdy_list_unlock();

        repeat:

        /* Lock the signal list */
        lock_acquire(&MANY_TLS(hthread)->sig_lock);

        /* While there are no signals left */
        while (!list_is_empty(&MANY_TLS(hthread)->sig_list)) {

            /* Get the signal */
            sig = list_dequeue(&MANY_TLS(hthread)->sig_list, Signal, sig_mem);

            /* Set the signal to the kernel thread */
            sig_send(KERNEL_THREAD_ID, sig->sig);

            /* Free the signal */
            free(sig);
        }

        /* Unlock the signal list */
        lock_release(&MANY_TLS(hthread)->sig_lock);

        /* Set the FS register value */
        set_fs(hthread);

        /* Start the timer */
        timer_start(&timer);

        /* Swap the context with the user thread */
        swapcontext(MANY_TLS(hthread)->ret_cxt, MANY_TLS(hthread)->curr_cxt);

        /* Stop the timer */
        timer_stop(&timer);

        /* Reset the FS register value to old value */
        set_fs(old_fs);

        /* Take action depending on the state */
        switch (hthread->state) {

            case HTHREAD_STATE_INIT:
            case HTHREAD_STATE_ACTIVE:

                /* Check if any signals are yet to be delivered */
                if (sig_is_pending()) {

                    goto repeat;
                }

                /* Lock the ready list */
                mm_rdy_list_lock();

                /* Add the current thread to the ready list */
                mm_rdy_list_add(hthread);

                /* Unlock the ready list */
                mm_rdy_list_unlock();
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
 * @brief Create a scheduler
 * @return Pointer to the scheduler instance
 */
static Scheduler *_mm_sched_create(void) {

    Scheduler *sched;

    /* Create a new scheduler */
    sched = alloc_mem(Scheduler);

    /* Allocate the stack */
    stack_alloc(&sched->stack);

    /* Create the kernel thread */
    sched->tid = clone(_mm_sched_dispatch,
                       sched->stack.ss_sp + sched->stack.ss_size,
                       _MM_SCHED_CLONE_FLAGS,
                       NULL,
                       &sched->wait,
                       NULL,
                       &sched->wait);
    /* Check for errors */
    assert(sched->tid != -1);

    return sched;
}

/**
 * @brief Destroy a scheduler
 * @param[in] sched Pointer to the scheduler instance
 */
static void _mm_sched_destroy(Scheduler *sched) {

    /* Check for errors */
    assert(sched);

    /* Wait for the kernel thread to finish */
    futex(&sched->wait, FUTEX_WAIT, sched->tid);

    /* Free the stack */
    stack_free(&sched->stack);

    /* Free the structure */
    free(sched);
}

/**
 * @brief Initialize the schedulers
 *
 * Creates specified number of schedulers. The schedulers will be
 * responsible for scheduling the many-many type of user threads.
 *
 * @param[in] nb_scheds Number of schedulers
 * @note Should be done by the main thread
 */
void mm_sched_init(int nb_scheds) {

    Scheduler *sched;

    /* Set the scheduling status */
    mm_sched_enabled = 1;

    /* For the requested number of schedulers */
    for (int i = 0; i < nb_scheds; i++) {

        /* Create a scheduler */
        sched = _mm_sched_create();

        /* Add the scheduler to the global list */
        list_enqueue(&mm_sched_list, sched, sched_mem);
    }
}

/**
 * @brief Deinitialize the many-many schedulers
 *
 * Frees the resources allocated to the scheduler. Also this function does
 * not return unless all the schedulers have not returned
 *
 * @note Should be done by the main thread
 */
void mm_sched_deinit(void) {

    Scheduler *sched;

    /* Clear the scheduling status */
    mm_sched_enabled = 0;

    /* While the scheduler list is not empty */
    while (!list_is_empty(&mm_sched_list)) {

        /* Get the scheduler from the global list */
        sched = list_dequeue(&mm_sched_list, Scheduler, sched_mem);

        /* Destroy the thread */
        _mm_sched_destroy(sched);
    }
}
