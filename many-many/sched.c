#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <syscall.h>
#include <asm/prctl.h>
#include <sys/prctl.h>

#include "./types.h"
#include "./stack.h"
#include "./timer.h"
#include "./list.h"
#include "./sched.h"

/* Schedulers states */
static Scheduler _scheds[NB_OF_SCHEDS];

/**
 * @brief Set FS register value
 * @param[in] addr Address to be set
 */
static inline void _set_fs(long addr) {

    /* Use the syscall wrapper */
    syscall(SYS_arch_prctl, ARCH_SET_FS, addr);
}

/**
 * @brief Get FS register value
 * @return Value of the FS register (long)
 */
static inline long _get_fs(void) {

    long addr;

    /* Use the syscall wrapper */
    syscall(SYS_arch_prctl, ARCH_GET_FS, &addr);

    return addr;
}

/**
 * @brief Yields the control from the user thread to the dispatcher
 * @param[in] argument Not considered
 */
static void _sched_yield(int argument) {

    Thread thread;

    /* Get the address of the thread local storage */
    thread = (Thread)_get_fs();

    /* Swap the context with the scheduler */
    swapcontext(&thread->curr_context, &thread->ret_context);
}

/**
 * @brief Dispatches a user thread onto a kernel thread
 * @param[in] argument Pointer to the scheduler instance
 * @return Integer
 */
static int _sched_dispatch(void *argument) {

    Scheduler *sched;
    Timer timer;
    Thread thread;
    long old_fs;

    /* Get the argument scheduler */
    sched = (Scheduler *)argument;

    /* Initialize the timer event */
    timer_set(&timer, _sched_yield, TIME_SLICE_ms);

    /* Get the value of the old fs register */
    old_fs = _get_fs();

    /* For eternity */
    while (1) {

        /* Lock the thread list */
        list_lock();

        /* If the list is empty */
        if (list_is_empty()) {

            /* Unlock and continue */
            list_unlock();
            continue;
        }

        /* Get the thread to be scheduled */
        thread = list_dequeue();

        /* Unlock the thread list */
        list_unlock();

        /* Set the currently mapped thread in scheduler */
        sched->thread = thread;

        /* Set the pointer to thread local storage */
        _set_fs((long)thread);

        /* Start the timer */
        timer_start(&timer);

        /* Swap the context with the target thread */
        swapcontext(&thread->ret_context, &thread->curr_context);

        /* Stop the timer */
        timer_stop(&timer);

        /* Reset the pointer to thread local storage */
        _set_fs(old_fs);

        /* Reset the currently mapped thread in scheduler */
        sched->thread = NULL;

        /* If the thread is still active */
        if (thread->state == THREAD_ACTIVE) {

            /* Lock the thread list */
            list_lock();

            /* Add the thread back to the list */
            list_enqueue(thread);

            /* Unlock the thread list */
            list_unlock();
        }
    }
}

/**
 * @brief Allocate a scheduler
 * @param[in] sched Pointer to the scheduler instance
 */
static void _sched_create(Scheduler *sched) {

    /* Set the currently mapped user thread to none */
    sched->thread = NULL;

    /* Allocate the stack for the scheduler */
    stack_alloc(&sched->stack);

    /* Create the kernel thread for running the scheduler */
    clone(_sched_dispatch,
          sched->stack.ss_sp + sched->stack.ss_size,
          CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND |
          CLONE_THREAD | CLONE_SYSVSEM | CLONE_PARENT_SETTID,
          sched,
          &sched->thread_id);
}

/**
 * @brief Deallocate a scheduler
 * @param[in] sched Pointer to the scheduler instance
 */
static void _sched_destroy(Scheduler *sched) {

}

/**
 * @brief Initializes the schedulers for the model
 */
void sched_init(void) {

    /* For each scheduler */
    for (int i = 0; i < NB_OF_SCHEDS; i++) {

        /* Create the scheduler */
        _sched_create(&_scheds[i]);
    }
}

/**
 * @brief Denitializes the schedulers for the model
 */
void sched_deinit(void) {

    /* For each scheduler */
    for (int i = 0; i < NB_OF_SCHEDS; i++) {

        /* Destroy the scheduler */
        _sched_destroy(&_scheds[i]);
    }
}