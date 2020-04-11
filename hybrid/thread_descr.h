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
} ThreadState;

/**
 * Thread control block / thread descriptor definition
 */
struct Thread {

    /* User thread id */
    int utid;

    /* Thread type */
    ThreadType type;

    /* Thread state */
    ThreadState state;

    /* Start routine */
    thread_start_t start;

    /* Argument */
    ptr_t arg;

    /* Return value */
    ptr_t ret;

    /* Stack canary */
    ptr_t __stack_canary;

    /* Wait word */
    int wait;

    /* Error number */
    int error;

    /* Waiting join thread descriptor */
    Thread join_td;

    /* Waiting for join thread descriptor */
    Thread wait_join_td;

    /* Lock for accessing members */
    Lock mem_lock;

    /* Type specific members */
    union {

        /* One-one thread members */
        struct {

            /* Kernel thread id */
            int ktid;

            /* Thread stack */
            stack_t stack;
        };

        /* Many-many thread members */
        struct {

            /* Current context*/
            ucontext_t *curr_cxt;

            /* Return context */
            ucontext_t *ret_cxt;

            /* Pending signal mask */
            int pend_sig;

            /* Disable timer interrupt */
            int intr_off;

            /* Timer object */
            Timer timer;

            /* Ready list links */
            ListMember ll_mem;
        };
    };
};

/**
 * Thread descriptor state handling
 */
#define td_set_state(thread, st)    ((thread)->state = (st))
#define td_get_state(thread)        ((thread)->state)
#define td_is_running(thread)       ((thread)->state == THREAD_STATE_RUNNING)
#define td_is_exited(thread)        ((thread)->state == THREAD_STATE_EXITED)
#define td_is_joined(thread)        ((thread)->state == THREAD_STATE_JOINED)
#define td_is_waiting(thread)       ((thread)->state == THREAD_STATE_WAIT_JOIN)

/**
 * Thread descriptor type checking
 */
#define td_is_one_one(thread)       ((thread)->type == THREAD_TYPE_ONE_ONE)
#define td_is_many_many(thread)     ((thread)->type == THREAD_TYPE_MANY_MANY)

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
#define td_oo_alloc()                   \
    ({                                          \
        Thread __td;                            \
                                                \
        /* Allocate the descriptor */           \
        __td = alloc_mem(struct Thread);        \
                                                \
        /* Allocate the stack */                \
        stack_alloc(&__td->stack);              \
                                                \
        /* Return the thread descriptor */      \
        __td;                                   \
    })
#define td_mm_alloc()                    \
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
#define td_oo_free(thread)                          \
    {                                               \
        /* Free the stack */                        \
        stack_free(&(thread)->stack);               \
                                                    \
        /* Free the descriptor */                   \
        /* free(thread); */                         \
    }
#define td_mm_free(thread)                          \
    {                                               \
        /* Free the stack */                        \
        stack_free(&(thread)->curr_cxt->uc_stack);  \
                                                    \
        /* Free the contexts */                     \
        free((thread)->curr_cxt);                   \
        free((thread)->ret_cxt);                    \
                                                    \
        /* Free the descriptor */                   \
        /* free(thread); */                         \
    }

/**
 * Thread descriptor base initialization
 */
#define td_oo_init(thread, id, st, ar)          \
    {                                           \
        /* Set the user thread id */            \
        (thread)->utid = (id);                  \
                                                \
        /* Set the thread type */               \
        (thread)->type = THREAD_TYPE_ONE_ONE;   \
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
        /* Set the join thread to none */       \
        (thread)->join_td = NULL;               \
                                                \
        /* Set the wait for object */           \
        (thread)->wait_join_td = NULL;          \
                                                \
        /* Initialize the member lock */        \
        lock_init(&(thread)->mem_lock);         \
    }
#define td_mm_init(thread, id, st, ar)          \
    {                                           \
    /* Set the user thread id */                \
        (thread)->utid = (id);                  \
                                                \
        /* Set the thread type */               \
        (thread)->type = THREAD_TYPE_MANY_MANY; \
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
        (thread)->join_td = NULL;               \
                                                \
        /* Set the wait for object */           \
        (thread)->wait_join_td = NULL;          \
                                                \
        /* Initialize the member lock */        \
        lock_init(&(thread)->mem_lock);         \
                                                \
        /* Set the pending signals */           \
        (thread)->pend_sig = 0;                 \
                                                \
        /* Set the interrupt handling status */ \
        (thread)->intr_off = 0;                 \
    }

/**
 * Thread descriptor creation handling
 */
#define td_oo_create(thread, func)                                  \
    {                                                               \
        /* Clone the requested function */                          \
        (thread)->ktid = clone(func,                                \
                               (thread)->stack.ss_sp +              \
                               (thread)->stack.ss_size,             \
                               CLONE_VM | CLONE_FS | CLONE_FILES |  \
                               CLONE_SIGHAND | CLONE_THREAD |       \
                               CLONE_SYSVSEM | CLONE_SETTLS |       \
                               CLONE_CHILD_CLEARTID |               \
                               CLONE_PARENT_SETTID,                 \
                               NULL,                                \
                               &(thread)->wait,                     \
                               (thread),                            \
                               &(thread)->wait);                    \
    }
#define td_mm_create(thread, func)                          \
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

/**
 * Thread descriptor many many context handling
 */
#define td_mm_set_cxt(thread)                               \
    {                                                       \
        /* Store the current context in return context      \
         * and set the context of the thread function */    \
        swapcontext((thread)->ret_cxt, (thread)->curr_cxt); \
    }
#define td_mm_ret_cxt(thread)                               \
    {                                                       \
        /* Store the current context in current context     \
         * and set the context of the return function */    \
        swapcontext((thread)->curr_cxt, (thread)->ret_cxt); \
    }
#define td_mm_exit_cxt(thread)                              \
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
 * Thread descriptor one one kernel thread id
 */
#define td_oo_get_ktid(thread)  ((thread)->ktid)

/**
 * Thread descriptor many many pending signals handling
 */
#define td_mm_set_sig_pending(thread, signo)        \
    ((thread)->pend_sig |= (1u << ((signo) - 1)))
#define td_mm_clear_sig_pending(thread, signo)      \
    ((thread)->pend_sig &= ~(1u << ((signo) - 1)))
#define td_mm_is_sig_pending(thread)  ((thread)->pend_sig != 0)
#define td_mm_get_sig_pending(thread)                   \
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
#define td_set_join(target_thread, calling_thread)                  \
    {                                                               \
        /* Set the calling thread as join thread in                 \
         * target thread */                                         \
        (target_thread)->join_td = (calling_thread);                \
                                                                    \
        /* Set target thread as the thread for which the calling    \
         * thread is waiting for */                                 \
        (calling_thread)->wait_join_td = (target_thread);           \
    }
#define td_get_join_thread(thread)        ((thread)->join_td)
#define td_has_join_thread(thread)        ((thread)->join_td != NULL)
#define td_get_wait_join_thread(thread)   ((thread)->wait_join_td)
#define td_clear_wait_join_thread(thread) ((thread)->wait_join_td = NULL)

/**
 * Thread descriptor many many interrupt handling
 */
#define td_mm_disable_intr(thread) ((thread)->intr_off = 1)
#define td_mm_enable_intr(thread)  ((thread)->intr_off = 0)
#define td_mm_is_intr_off(thread)  ((thread)->intr_off)

/**
 * Thread descriptor timer handling
 */
#define td_mm_timer_init(thread, act, tms)      \
    (timer_set(&(thread)->timer, act, tms))
#define td_mm_timer_start(thread)        (timer_start(&(thread)->timer))
#define td_mm_timer_stop(thread)         (timer_stop(&(thread)->timer))

/**
 * Thread descriptor exclusive access handling
 */
#define td_lock(thread)    (lock_acquire(&(thread)->mem_lock))
#define td_unlock(thread)  (lock_release(&(thread)->mem_lock))


#endif
