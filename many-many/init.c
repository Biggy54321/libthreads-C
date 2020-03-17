#include <stdlib.h>
#include <unistd.h>
#include <syscall.h>
#include <asm/prctl.h>
#include <sys/prctl.h>

#include "./lock.h"
#include "./stack.h"
#include "./init.h"

/* Thread id to be assigned to next admitted thread */
static int _nxt_tid = 0;
static Lock _lock = LOCK_NOT_ACQUIRED;

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
 * @brief Actual thread start function
 */
static void _thread_start(void) {

    Thread thread;

    /* Get the thread handle */
    thread = (Thread)_get_fs();

    /* Call the requested thread start routine */
    thread->return_value = thread->start_routine(thread->argument);

    /* Mark the thread dead */
    thread->state = THREAD_DEAD;
}

/**
 * @brief Initialize a thread
 * @param[out] thread Pointer to thread handle
 * @param[in] start_routine Start function of the thread
 * @param[in] argument Argument to the start function
 */
void init_thread(
        Thread *thread,
        void *(*start_routine)(void *),
        void *argument) {

    /* Create a new thread */
    *thread = (Thread)malloc(THREAD_CONTROL_BLOCK_SIZE);

    /* Lock the next thread id */
    lock_acquire(&_lock);

    /* Set the thread id */
    (*thread)->thread_id = _nxt_tid++;

    /* Unlock the next thread id */
    lock_release(&_lock);

    /* Set the state */
    (*thread)->state = THREAD_ACTIVE;

    /* Set the start routine */
    (*thread)->start_routine = start_routine;

    /* Set the start routine argument */
    (*thread)->argument = argument;

    /* Get the current context */
    getcontext(&(*thread)->curr_context);

    /* Allocate the stack */
    stack_alloc(&((*thread)->curr_context.uc_stack));

    /* Set the backlink */
    (*thread)->curr_context.uc_link = &(*thread)->ret_context;

    /* Make the context */
    makecontext(&(*thread)->curr_context, _thread_start, 0);
}
