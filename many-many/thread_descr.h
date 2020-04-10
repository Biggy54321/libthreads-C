#ifndef _THREAD_DESCR_H_
#define _THREAD_DESCR_H_

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
typedef enum {

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
} ThreadState;

/**
 * Thread control block / thread descriptor definition
 */
struct Thread {

    /* User thread id */
    int utid;

    /* Thread state */
    ThreadState state;

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

    /* Pointer to join waiting thread descriptor */
    struct Thread *join_thread;

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
#define TD_SET_STATE(thread, st)    ((thread)->state = (st))
#define TD_GET_STATE(thread)        ((thread)->state)
#define TD_IS_RUNNING(thread)       ((thread)->state == THREAD_STATE_RUNNING)
#define TD_IS_EXITED(thread)        ((thread)->state == THREAD_STATE_EXITED)
#define TD_IS_JOINED(thread)        ((thread)->state == THREAD_STATE_JOINED)
#define TD_IS_WAITING(thread)                           \
    (((thread)->state == THREAD_STATE_WAIT_JOIN) ||     \
     ((thread)->state == THREAD_STATE_WAIT_MUTEX))

/**
 * Thread descriptor launch
 */
#define TD_LAUNCH(thread) ((thread)->ret = (thread)->start(thread->arg))

/**
 * Thread descriptor return value handling
 */
#define TD_SET_RET(thread, ret_val) ((thread)->ret = (ret_val))
#define TD_GET_RET(thread)          ((thread)->ret)

/**
 * Thread descriptor memory allocation
 */
#define TD_ALLOC()                              \
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
#define TD_FREE(thread)                             \
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
#define TD_INIT(thread, id, st, ar)             \
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
        /* Initialize the member lock */        \
        lock_init(&(thread)->mem_lock);         \
    }

/**
 * Thread descriptor context handling
 */
#define TD_INIT_CXT(thread, func)                           \
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
#define TD_SET_CXT(thread)                                  \
    {                                                       \
        /* Store the current context in return context      \
         * and set the context of the thread function */    \
        swapcontext((thread)->ret_cxt, (thread)->curr_cxt); \
    }
#define TD_RET_CXT(thread)                                  \
    {                                                       \
        /* Store the current context in current context     \
         * and set the context of the return function */    \
        swapcontext((thread)->curr_cxt, (thread)->ret_cxt); \
    }
#define TD_EXIT_CXT(thread)                                 \
    {                                                       \
        /* Don't save the current context just return to    \
         * whatever return context already set */           \
        setcontext((thread)->ret_cxt);                      \
    }

/**
 * Thread descriptor wait/completion handling
 */
#define TD_IS_OVER(thread)  (!(thread)->wait)
#define TD_SET_OVER(thread) ((thread)->wait = 0)

/**
 * Thread descriptor pending signals handling
 */
#define TD_SET_SIG_PENDING(thread, signo)           \
    ((thread)->pend_sig |= (1u << ((signo) - 1)))
#define TD_CLEAR_SIG_PENDING(thread, signo)         \
    ((thread)->pend_sig &= ~(1u << ((signo) - 1)))
#define TD_IS_SIG_PENDING(thread)  ((thread)->pend_sig != 0)
#define TD_GET_SIG_PENDING(thread)                      \
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
#define TD_GET_JOINING(thread)  ((thread)->join_thread)
#define TD_HAS_JOINING(thread)  ((thread)->join_thread != NULL)
#define TD_SET_JOINING(thread, join_thd)        \
    ((thread)->join_thread = (join_thd))

/**
 * Thread descriptor interrupt handling
 */
#define TD_DISABLE_INTR(thread) ((thread)->intr_off = 1)
#define TD_ENABLE_INTR(thread)  ((thread)->intr_off = 0)
#define TD_IS_INTR_OFF(thread)  ((thread)->intr_off)

/**
 * Thread descriptor timer handling
 */
#define TD_TIMER_INIT(thread, act, tms) (timer_set(&(thread)->timer, act, tms))
#define TD_TIMER_START(thread)          (timer_start(&(thread)->timer))
#define TD_TIMER_STOP(thread)           (timer_stop(&(thread)->timer))

/**
 * Thread descriptor exclusive access handling
 */
#define TD_LOCK(thread)    (lock_acquire(&(thread)->mem_lock))
#define TD_UNLOCK(thread)  (lock_release(&(thread)->mem_lock))

#endif
