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
#define MMSCHED_CLONE_FLAGS                     \
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

    /* If interrupts are disabled */
    if (TD_IS_INTR_OFF(thread)) {

        /* Stop the current timer */
        TD_TIMER_STOP(thread);

        /* Restart the timer */
        TD_TIMER_START(thread);

        return;
    }

    /* Swap the context with the dispatcher */
    TD_RET_CXT(thread);
}

/**
 * Action for the timer interrupt
 */
#define TIMER_INTR_ACTION                       \
    ({                                          \
        struct sigaction __action;              \
                                                \
        /* Initialize the action */             \
        __action.sa_handler = _mmsched_yield;   \
        __action.sa_flags = 0;                  \
        sigfillset(&__action.sa_mask);          \
                                                \
        /* Return the initialized action */     \
        __action;                               \
    })

/* Repeatation label name */
#define REPEAT_LABEL repeat

/**
 * Get the next ready thread
 * @note This macro should be used in some kind of loop, as it employs a use
 *       continue statement. It will be blocking till it gets a ready thread
 *       on the list
 */
#define GET_NEXT_THREAD(thread)                 \
  {                                             \
      /* Lock the ready list */                 \
      mmrll_lock();                             \
                                                \
      /* If the ready list is empty */          \
      if (mmrll_is_empty()) {                   \
                                                \
          /* Unlock the list and continue */    \
          mmrll_unlock();                       \
          continue;                             \
      }                                         \
                                                \
      /* Get a thread from the list */          \
      (thread) = mmrll_dequeue();               \
                                                \
      /* Unlock the ready list */               \
      mmrll_unlock();                           \
  }

/**
 * Sends all the pending signals of the thread
 */
#define SEND_PENDING_SIGNALS(thread)                \
  {                                                 \
      int __signo;                                  \
                                                    \
      /* Lock the thread descriptor */              \
      TD_LOCK(thread);                              \
                                                    \
      /* While there are no signals to be sent */   \
      while (TD_IS_SIG_PENDING(thread)) {           \
                                                    \
          /* Get the signal number */               \
          __signo = TD_GET_SIG_PENDING(thread);     \
                                                    \
          /* Send the signal */                     \
          sig_send(KERNEL_THREAD_ID, __signo);      \
      }                                             \
                                                    \
      /* Unlock the thread descriptor */            \
      TD_UNLOCK(thread);                            \
  }

/**
 * Post schedule running state action
 */
#define POST_SCHEDULE_RUNNING_ACTION(thread)                \
    {                                                       \
        /* Check if any signals are yet to be delivered */  \
        if (sig_is_pending()) {                             \
                                                            \
            goto REPEAT_LABEL;                              \
        }                                                   \
                                                            \
        /* Lock the ready list */                           \
        mmrll_lock();                                       \
                                                            \
        /* Add the current thread to the ready list */      \
        mmrll_enqueue(thread);                              \
                                                            \
        /* Unlock the ready list */                         \
        mmrll_unlock();                                     \
    }

/**
 * Post schedule exited state action
 */
#define POST_SCHEDULE_EXITED_ACTION(thread)                             \
    {                                                                   \
        /* Acquire the member lock */                                   \
        TD_LOCK(thread);                                                \
                                                                        \
        /* Clear the wait state */                                      \
        TD_SET_OVER(thread);                                            \
                                                                        \
        /* Check if the thread has thread waiting to join */            \
        if (TD_HAS_JOINING(thread)) {                                   \
                                                                        \
            /* Lock the ready list */                                   \
            mmrll_lock();                                               \
                                                                        \
            /* Add the current thread to the ready list */              \
            mmrll_enqueue(TD_GET_JOINING(thread));                      \
                                                                        \
            /* Unlock the ready list */                                 \
            mmrll_unlock();                                             \
        }                                                               \
                                                                        \
        /* Release the member lock */                                   \
        TD_UNLOCK(thread);                                              \
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
    void *old_fs;

    /* Block all the signals */
    sig_block_all();

    /* Get the current FS register value */
    old_fs = get_fs();

    /* While the scheduling is enabled */
    while (mmsched_enabled) {

        /* Get a thread to be scheduled */
        GET_NEXT_THREAD(thread);

        /* If the thread state is running */
        if (TD_IS_RUNNING(thread)) {

            /* Send the pending signals */
            SEND_PENDING_SIGNALS(thread);
        }

        /* Initialize the timer */
        TD_TIMER_INIT(thread, TIMER_INTR_ACTION, MMSCHED_TIME_SLICE_ms);

        REPEAT_LABEL:

        /* Set the FS register value */
        set_fs(thread);

        /* Start the timer */
        TD_TIMER_START(thread);

        /* Swap the context with the user thread */
        TD_SET_CXT(thread);

        /* Stop the timer */
        TD_TIMER_STOP(thread);

        /* Reset the FS register value to old value */
        set_fs(old_fs);

        /* Take action depending on the state */
        switch (TD_GET_STATE(thread)) {

            case THREAD_STATE_RUNNING:

                /* Carry the post schedule running action */
                POST_SCHEDULE_RUNNING_ACTION(thread);
                break;

            case THREAD_STATE_WAIT_JOIN:

                /* Release the lock of the wait for thread */
                TD_UNLOCK(TD_GET_WAIT_THREAD(thread));

                break;

            case THREAD_STATE_WAIT_MUTEX:

                /*  */

                break;

            case THREAD_STATE_EXITED:

                /* Carry the post schedule exited action */
                POST_SCHEDULE_EXITED_ACTION(thread);
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
    /* Check for errors */
    assert(sched);

    /* Allocate the stack */
    stack_alloc(&sched->stack);

    /* Create the kernel thread */
    sched->ktid = clone(_mmsched_dispatch,
                        sched->stack.ss_sp + sched->stack.ss_size,
                        MMSCHED_CLONE_FLAGS,
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
