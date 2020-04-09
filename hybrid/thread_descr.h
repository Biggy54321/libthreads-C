#ifndef _THREAD_DESCR_H_
#define _THREAD_DESCR_H_

#include <ucontext.h>
#include <signal.h>

#include "./mods/list.h"
#include "./mods/lock.h"
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

    /* Thread is waiting for spinlock */
    THREAD_STATE_WAIT_SPINLOCK
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

    /* Pointer to join waiting thread descriptor */
    struct Thread *join_thread;

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

            /* Ready list links */
            ListMember mmrll_mem;
        };
    };
};

#endif
