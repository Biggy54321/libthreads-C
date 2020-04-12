#ifndef _THREAD_DESCR_H_
#define _THREAD_DESCR_H_

#define _GNU_SOURCE
#include <sched.h>
#include <ucontext.h>
#include <signal.h>
#include <string.h>

#include "./mods/utils.h"
#include "./mods/stack.h"
#include "./mods/lock.h"
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

    /* Thread is waiting to acquire the mutex */
    THREAD_STATE_WAIT_MUTEX,

    /* Thread is waiting on a condition variable */
    THREAD_STATE_WAIT_COND

};

/**
 * Thread control block / thread descriptor definition
 */
struct Thread {

    /* Kernel thread id */
    int ktid;

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

    /* Error number */
    int error;

    /* Stack canary */
    ptr_t __stack_canary;

    /* Waiting join thread descriptor */
    Thread join_td;

    /* Lock for accessing members */
    Lock mem_lock;

    /* Thread stack */
    stack_t stack;
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
     ((thread)->state == THREAD_STATE_WAIT_MUTEX) ||    \
     ((thread)->state == THREAD_STATE_WAIT_COND))

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
        /* Allocate the stack */                \
        stack_alloc(&__td->stack);              \
                                                \
        /* Return the thread descriptor */      \
        __td;                                   \
    })

/**
 * Thread descriptor memory free
 */
#define td_free(thread)                         \
    {                                           \
        /* Free the stack */                    \
        stack_free(&(thread)->stack);           \
    }

/**
 * Thread descriptor base initialization
 */
#define td_init(thread, st, ar)                 \
    {                                           \
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
        /* Initialize the member lock */        \
        lock_init(&(thread)->mem_lock);         \
    }

/**
 * Thread descriptor creation handling
 */
#define td_create(thread, func)                                     \
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

/**
 * Thread descriptor wait/completion handling
 */
#define td_is_over(thread)  (!(thread)->wait)

/**
 * Thread descriptor kernel thread id
 */
#define td_get_ktid(thread)  ((thread)->ktid)


/**
 * Thread descriptor joining thread handling
 */
#define td_set_join_thread(target_thread, call_thread)  \
    ((target_thread)->join_td = (call_thread))
#define td_get_join_thread(thread)        ((thread)->join_td)
#define td_has_join_thread(thread)        ((thread)->join_td != NULL)

/**
 * Thread descriptor exclusive access handling
 */
#define td_lock(thread)    (lock_acquire(&(thread)->mem_lock))
#define td_unlock(thread)  (lock_release(&(thread)->mem_lock))

#endif
