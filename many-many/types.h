#ifndef _TYPES_H_
#define _TYPES_H_

#include <ucontext.h>
#include <time.h>
#include <signal.h>

/**
 * Thread state enumeration
 */
typedef enum _ThreadState {

    /* Thread is running (i.e. schedulable) */
    THREAD_ACTIVE,

    /* Thread is done executing */
    THREAD_DEAD

} ThreadState;

/**
 * Thread control block structure
 */
typedef struct _ThreadControlBlock {

    /* User thread id */
    int thread_id;

    /* Thread state */
    ThreadState state;

    /* Thread start routine */
    void *(*start_routine)(void *);

    /* Thread argument */
    void *argument;

    /* Thread return value */
    void *return_value;

    /* Padding for stack canary */
    long _pad[2];

    /* Thread current context */
    ucontext_t curr_context;

    /* Thread return context */
    ucontext_t ret_context;

    /* Forward link */
    struct _ThreadControlBlock *next;

    /* Backward link */
    struct _ThreadControlBlock *prev;

} ThreadControlBlock;

/**
 * Size of thread control block in bytes
 */
#define THREAD_CONTROL_BLOCK_SIZE (sizeof(ThreadControlBlock))

/**
 * Thread handle
 */
typedef ThreadControlBlock *Thread;

/**
 * Lock
 */
typedef int Lock;

/**
 * Lock status
 */
#define LOCK_ACQUIRED     (0u)
#define LOCK_NOT_ACQUIRED (1u)

/**
 * One shot timer
 */
typedef struct _Timer {

    /* Timer structure */
    timer_t timerid;

    /* Interval timeout structure */
    struct itimerspec interval;

    /* Signal event structure */
    struct sigevent event;

} Timer;

/**
 * Scheduler
 */
typedef struct _Scheduler {

    /* Kernel thread id of the scheduler */
    int thread_id;

    /* Stack for the scheduler */
    stack_t stack;

    /* Currently mapped user thread handle */
    Thread thread;

} Scheduler;

#endif
