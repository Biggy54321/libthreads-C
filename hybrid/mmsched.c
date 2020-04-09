#define _GNU_SOURCE
#include <sched.h>
#include <string.h>

#include "./mods/utils.h"
#include "./mods/list.h"
#include "./mods/lock.h"
#include "./mods/stack.h"
#include "./mods/sig.h"
#include "./mods/timer.h"
#include "./mmrll.h"
#include "./mmsched.h"
#include "./thread.h"
#include "./thread_descr.h"

/* Clone flags for the kernel thread of the scheduler */
#define _MMSCHED_CLONE_FLAGS                    \
    (CLONE_VM | CLONE_FS | CLONE_FILES |        \
     CLONE_SIGHAND | CLONE_THREAD |             \
     CLONE_SYSVSEM | CLONE_PARENT_SETTID |      \
     CLONE_CHILD_CLEARTID)

/* Scheduler list */
static List mmsched_list;
/* Scheduling status */
static int mmsched_enabled;

/**
 * @brief Yield the control to the dispatcher from the user thread
 * @param[in] arg Not used
 */
static void _mmsched_yield(int arg) {

    Thread thread;

    /* Get the thread handle */
    thread = thread_self();

    /* Swap the context with the dispatcher */
    swapcontext(thread->curr_cxt, thread->ret_cxt);
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
static int _mmsched_dispatch(void *arg) {

    Thread thread;
    Timer timer;
    void *old_fs;
    int signo;
    struct sigaction act;

    /* Block all the signals */
    sig_block_all();

    /* Initialize the action for the timer event */
    act.sa_handler = _mmsched_yield;
    act.sa_flags = 0;
    sigfillset(&act.sa_mask);

    /* Set the timer for the above action */
    timer_set(&timer, act, MMSCHED_TIME_SLICE_ms);

    /* Get the current FS register value */
    old_fs = get_fs();

    /* While the scheduling is enabled */
    while (mmsched_enabled) {

        /* Lock the ready list */
        mmrll_lock();

        /* If the ready list is empty */
        if (mmrll_is_empty()) {

            /* Unlock the list and continue */
            mmrll_unlock();
            continue;
        }

        /* Get a thread from the list */
        thread = mmrll_dequeue();

        /* Unlock the ready list */
        mmrll_unlock();

        /* Lock the signal list */
        lock_acquire(&thread->mem_lock);

        /* While there are no signals to be sent */
        while (thread->pend_sig) {

            /* Get the signal number */
            signo = ffs(thread->pend_sig);

            /* Mask the signal */
            thread->pend_sig ^= (1 << (signo - 1));

            /* Send the signal */
            sig_send(KERNEL_THREAD_ID, signo);
        }

        /* Unlock the signal list */
        lock_release(&thread->mem_lock);

        repeat:

        /* Set the FS register value */
        set_fs(thread);

        /* Start the timer */
        timer_start(&timer);

        /* Swap the context with the user thread */
        swapcontext(thread->ret_cxt, thread->curr_cxt);

        /* Stop the timer */
        timer_stop(&timer);

        /* Reset the FS register value to old value */
        set_fs(old_fs);

        /* Take action depending on the state */
        switch (thread->state) {

            case THREAD_STATE_RUNNING:
            case THREAD_STATE_WAIT_JOIN:
            case THREAD_STATE_WAIT_SPINLOCK:

                /* Check if any signals are yet to be delivered */
                if (sig_is_pending()) {

                    goto repeat;
                }

                /* Lock the ready list */
                mmrll_lock();

                /* Add the current thread to the ready list */
                mmrll_enqueue(thread);

                /* Unlock the ready list */
                mmrll_unlock();
                break;

            case THREAD_STATE_EXITED:

                /* Clear the wait state */
                thread->wait = 0;
                break;

            default:
                break;
        }
    }

    return 0;
}

/**
 * @brief Create a scheduler
 * @return Pointer to the scheduler instance
 */
static Scheduler *_mmsched_create(void) {

    Scheduler *sched;

    /* Create a new scheduler */
    sched = alloc_mem(Scheduler);

    /* Allocate the stack */
    stack_alloc(&sched->stack);

    /* Create the kernel thread */
    sched->ktid = clone(_mmsched_dispatch,
                        sched->stack.ss_sp + sched->stack.ss_size,
                        _MMSCHED_CLONE_FLAGS,
                        NULL,
                        &sched->wait,
                        NULL,
                        &sched->wait);

    /* Check for errors */
    assert(sched->ktid != -1);

    return sched;
}

/**
 * @brief Destroy a scheduler
 * @param[in] sched Pointer to the scheduler instance
 */
static void _mmsched_destroy(Scheduler *sched) {

    /* Check for errors */
    assert(sched);

    /* Wait for the kernel thread to finish */
    futex(&sched->wait, FUTEX_WAIT, sched->ktid);

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
void mmsched_init(int nb_scheds) {

    Scheduler *sched;

    /* Initialize the list */
    list_init(&mmsched_list);

    /* Set the scheduling status */
    mmsched_enabled = 1;

    /* For every requested scheduler */
    for (int i = 0; i < nb_scheds; i++) {

        /* Create a scheduler instance */
        sched = _mmsched_create();

        /* Add the scheduler to the list */
        list_enqueue(&mmsched_list, sched, sll_mem);
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
void mmsched_deinit(void) {

    Scheduler *sched;

    /* Clear the scheduling status */
    mmsched_enabled = 0;

    /* While the scheduler list is not empty */
    while (!list_is_empty(&mmsched_list)) {

        /* Get the scheduler from the global list */
        sched = list_dequeue(&mmsched_list, Scheduler, sll_mem);

        /* Destroy the thread */
        _mmsched_destroy(sched);
    }
}
