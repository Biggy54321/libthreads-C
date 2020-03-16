#ifndef _USER_THREAD_H_
#define _USER_THREAD_H_

#include <ucontext.h>

/**
 * User thread control structure
 */
typedef struct _UserThreadControlBlock {

    /* Thread id */
    int thread_id;

    /* Thread start function */
    void *(*start_routine)(void *);

    /* Start function argument */
    void *argument;

    /* Thread return value */
    void *return_value;

    /* Wait word */
    int wait_word;

    /* Context of the user thread */
    ucontext_t context;

    /* Forward link in the queue */
    struct _UserThreadControlBlock *next;

    /* Backward link in the queue */
    struct _UserThreadControlBlock *prev;

} UserThreadControlBlock;

/**
 * User thread handle
 */
typedef UserThreadControlBlock *UserThread;

#endif
