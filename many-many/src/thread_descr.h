#ifndef _THREAD_DESCR_H_
#define _THREAD_DESCR_H_

#define _GNU_SOURCE
#include <sched.h>
#include <ucontext.h>
#include <signal.h>
#include <string.h>

#include "./mods/utils.h"
#include "./mods/stack.h"
#include "./mods/list.h"
#include "./mods/lock.h"
#include "./mods/timer.h"
#include "./thread.h"

/**
 * Thread states
 */
enum {

    /* Thread is running */
    THREAD_STATE_RUNNING,

    /* Thread has exited */
    THREAD_STATE_EXITED,

    /* Thread has joined */
    THREAD_STATE_JOINED,

    /* Thread is waiting to join */
    THREAD_STATE_WAIT_JOIN,

    /* Thread is waiting for mutex */
    THREAD_STATE_WAIT_MUTEX
};

/**
 * Thread control block / thread descriptor definition
 */
struct Thread {

    /* User thread id */
    int utid;

    /* Thread state */
    int state;

    /* Start routine */
    thread_start_t start;

    /* Argument */
    ptr_t arg;

    /* Return value */
    ptr_t ret;

    /* Wait word */
    int wait;

    /* Pending signal mask */
    int pend_sig;

    /* Stack canary */
    ptr_t __stack_canary;

    /* Current context*/
    ucontext_t *curr_cxt;

    /* Return context */
    ucontext_t *ret_cxt;

    /* List links */
    ListMember ll_mem;

    /* Error number */
    int error;

    /* Pointer to the thread waiting for current thread to join */
    Thread join_thread;

    /* Pointer to the object the current thread is waiting for
     * This can be -
     * 1. Another thread
     * 2. Mutex  */
    ptr_t wait_for;

    /* Disable timer interrupt */
    int intr_off;

    /* Timer object */
    Timer timer;

    /* Lock for accessing members */
    Lock mem_lock;
};

/**
 * Thread descriptor state handling
 */
#define td_set_state(thread, st)    ((thread)->state = (st))
#define td_get_state(thread)        ((thread)->state)
#define td_is_running(thread)       ((thread)->state == THREAD_STATE_RUNNING)
#define td_is_exited(thread)        ((thread)->state == THREAD_STATE_EXITED)
#define td_is_joined(thread)        ((thread)->state == THREAD_STATE_JOINED)
#define td_is_waiting(thread)                           \
    (((thread)->state == THREAD_STATE_WAIT_JOIN) ||     \
     ((thread)->state == THREAD_STATE_WAIT_MUTEX))

/**
 * Thread descriptor launch
 */
#define td_launch(thread) ((thread)->ret = (thread)->start(thread->arg))

/**
 * Thread descriptor return value handling
 */
#define td_set_ret(thread, ret_val) ((thread)->ret = (ret_val))
#define td_get_ret(thread)          ((thread)->ret)

/**
 * Thread descriptor memory allocation
 */
#define td_alloc()                              \
    ({                                          \
        Thread __td;                            \
                                                \
        /* Allocate the descriptor */           \
        __td = alloc_mem(struct Thread);        \
                                                \
        /* Allocate the contexts */             \
        __td->curr_cxt = alloc_mem(ucontext_t); \
        __td->ret_cxt = alloc_mem(ucontext_t);  \
                                                \
        /* Allocate the stack */                \
        stack_alloc(&__td->curr_cxt->uc_stack); \
                                                \
        /* Return the thread descriptor */      \
        __td;                                   \
    })

/**
 * Thread descriptor memory free
 */
#define td_free(thread)                             \
    {                                               \
        /* Free the stack */                        \
        stack_free(&(thread)->curr_cxt->uc_stack);  \
                                                    \
        /* Free the contexts */                     \
        free((thread)->curr_cxt);                   \
        free((thread)->ret_cxt);                    \
                                                    \
        /* Free the descriptor */                   \
        free(thread);                               \
    }

/**
 * Thread descriptor base initialization
 */
#define td_init(thread, id, st, ar)             \
    {                                           \
        /* Set the user thread id */            \
        (thread)->utid = (id);                  \
                                                \
        /* Set the thread state */              \
        (thread)->state = THREAD_STATE_RUNNING; \
                                                \
        /* Set the start routine */             \
        (thread)->start = (st);                 \
                                                \
        /* Set the argument */                  \
        (thread)->arg = (ar);                   \
                                                \
        /* Initialize the wait word */          \
        (thread)->wait = 1;                     \
                                                \
        /* Set the join thread to none */       \
        (thread)->join_thread = NULL;           \
                                                \
        /* Set the interrupt handling status */ \
        (thread)->intr_off = 0;                 \
                                                \
        /* Set the pending signals */           \
        (thread)->pend_sig = 0;                 \
                                                \
        /* Set the wait for object */           \
        (thread)->wait_for = NULL;              \
                                                \
        /* Initialize the member lock */        \
        lock_init(&(thread)->mem_lock);         \
    }

/**
 * Thread descriptor context handling
 */
#define td_init_cxt(thread, func)                           \
    {                                                       \
        /* Get the current context */                       \
        getcontext((thread)->curr_cxt);                     \
                                                            \
        /* Set the back link */                             \
        (thread)->curr_cxt->uc_link = (thread)->ret_cxt;    \
                                                            \
        /* Make the context of the given function */        \
        makecontext((thread)->curr_cxt, func, 0);           \
    }
#define td_set_cxt(thread)                                  \
    {                                                       \
        /* Store the current context in return context      \
         * and set the context of the thread function */    \
        swapcontext((thread)->ret_cxt, (thread)->curr_cxt); \
    }
#define td_ret_cxt(thread)                                  \
    {                                                       \
        /* Store the current context in current context     \
         * and set the context of the return function */    \
        swapcontext((thread)->curr_cxt, (thread)->ret_cxt); \
    }
#define td_exit_cxt(thread)                                 \
    {                                                       \
        /* Don't save the current context just return to    \
         * whatever return context already set */           \
        setcontext((thread)->ret_cxt);                      \
    }

/**
 * Thread descriptor wait/completion handling
 */
#define td_is_over(thread)  (!(thread)->wait)
#define td_set_over(thread) ((thread)->wait = 0)

/**
 * Thread descriptor pending signals handling
 */
#define td_set_sig_pending(thread, signo)           \
    ((thread)->pend_sig |= (1u << ((signo) - 1)))
#define td_clear_sig_pending(thread, signo)         \
    ((thread)->pend_sig &= ~(1u << ((signo) - 1)))
#define td_is_sig_pending(thread)  ((thread)->pend_sig != 0)
#define td_get_sig_pending(thread)                      \
    ({                                                  \
        int __signo;                                    \
                                                        \
        /* Get the first pending signal */              \
        __signo = ffs((thread)->pend_sig);              \
                                                        \
        /* Mask the signo from the pending list */      \
        (thread)->pend_sig ^= (1u << (__signo - 1));    \
                                                        \
        /* Return the pending signal number */          \
        __signo;                                        \
    })

/**
 * Thread descriptor joining thread handling
 */
#define td_get_joining(thread)  ((thread)->join_thread)
#define td_has_joining(thread)  ((thread)->join_thread != NULL)
#define td_set_joining(thread, join_thd)        \
    ((thread)->join_thread = (join_thd))

/**
 * Thread descriptor wait objects handling
 */
#define td_set_wait_thread(thread, td)  ((thread)->wait_for = (td))
#define td_set_wait_mutex(thread, mut)  ((thread)->wait_for = (mut))
#define td_get_wait_thread(thread)      ((Thread)((thread)->wait_for))
#define td_get_wait_mutex(thread)       ((ThreadMutex)((thread)->wait_for))

/**
 * Thread descriptor interrupt handling
 */
#define td_disable_intr(thread) ((thread)->intr_off = 1)
#define td_enable_intr(thread)  ((thread)->intr_off = 0)
#define td_is_intr_off(thread)  ((thread)->intr_off)

/**
 * Thread descriptor timer handling
 */
#define td_timer_init(thread, act, tms) (timer_set(&(thread)->timer, act, tms))
#define td_timer_start(thread)          (timer_start(&(thread)->timer))
#define td_timer_stop(thread)           (timer_stop(&(thread)->timer))

/**
 * Thread descriptor exclusive access handling
 */
#define td_lock(thread)    (lock_acquire(&(thread)->mem_lock))
#define td_unlock(thread)  (lock_release(&(thread)->mem_lock))

#endif
