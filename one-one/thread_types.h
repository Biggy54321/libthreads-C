#ifndef _THREAD_TYPES_H_
#define _THREAD_TYPES_H_

#include <stdint.h>

#include "./mods/lock.h"

/**
 * Pointer definition
 */
typedef void *ptr_t;

/**
 * Starting thread routine
 */
typedef void *(*thread_start_t)(void *);

/**
 * Thread state
 */
typedef enum _ThreadState {

    /* Thread is running */
    THREAD_STATE_RUNNING,

    /* Thread has exited */
    THREAD_STATE_EXITED,

    /* Thread is joined */
    THREAD_STATE_JOINED,

    /* Thread is waiting for some thread to join */
    THREAD_STATE_WAIT_JOIN,

    /* Thread is waiting to acquire the spinlock */
    THREAD_STATE_WAIT_SPINLOCK,

    /* Thread is waiting to acquire the mutex */
    THREAD_STATE_WAIT_MUTEX,

    /* Thread is waiting on a condition variable */
    THREAD_STATE_WAIT_COND
} ThreadState;

/**
 * Thread control block definition
 * @note The structure members are placed so as to prevent any padding
 *       inserted by the compiler
 */
typedef struct _ThreadControlBlock {

    /* Stack base pointer */
    ptr_t stack_base;

    /* Stack size */
    uint64_t stack_limit;

    /* Thread start routine */
    thread_start_t start_routine;

    /* Thread argument */
    ptr_t argument;

    /* Thread return value */
    ptr_t return_value;

    /* Padding for stack canary */
    uint64_t __pad;

    /* Thread identifier */
    uint32_t thread_id;

    /* Join word */
    uint32_t join_word;

    /* State of the thread */
    ThreadState thread_state;

    /* Thread handle of the thread waiting on the current thread to join */
    struct _ThreadControlBlock *join_thread;

    /* Thread member lock */
    Lock mem_lock;

} ThreadControlBlock;

/**
 * Size of thread control block
 */
#define THREAD_CONTROL_BLOCK_SIZE (sizeof(ThreadControlBlock))

/**
 * Thread handle for user handling
 */
typedef ThreadControlBlock *Thread;

/**
 * Return statuses of the thread library
 */
typedef enum _ThreadReturn {

    THREAD_OK,                  /* The thread function was successful */
    THREAD_FAIL,                /* The thread function was not successful */

} ThreadReturn;

#endif
