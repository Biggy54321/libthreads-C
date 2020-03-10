#ifndef _THREAD_TYPES_H_
#define _THREAD_TYPES_H_

#include <stdint.h>

/**
 * Pointer definition
 */
typedef void *ptr_t;

/**
 * Starting thread routine
 */
typedef void *(*thread_start_t)(void *);

/**
 * Thread control block definition
 */
typedef struct _ThreadControlBlock {

    /* Thread identifier */
    uint32_t thread_id;

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

    /* Futex word */
    int32_t futex_word;

} ThreadControlBlock;

/**
 * Size of thread control block
 */
#define THREAD_CONTROL_BLOCK_SIZE (sizeof(ThreadControlBlock))

/**
 * Return statuses of the thread library
 */
typedef enum _ThreadReturn {

    THREAD_OK,                  /* The thread function was successful */
    THREAD_FAIL,                /* The thread function was not successful */

} ThreadReturn;

/**
 * Thread handle for user handling
 */
typedef ThreadControlBlock *Thread;

#endif
