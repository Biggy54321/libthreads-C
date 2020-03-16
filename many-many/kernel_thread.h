#ifndef _KERNEL_THREAD_H_
#define _KERNEL_THREAD_H_

#include <ucontext.h>

#include "./user_thread.h"

/**
 * Maximum number of kernel threads
 */
#define NB_OF_KERNEL_THREADS (2u)

/**
 * Kernel thread control structure
 */
typedef struct _KernelThreadControlBlock {

    /* Kernel thread id */
    int thread_id;

    /* Padding */
    int _pad1;

    /* Currently mapped user thread */
    UserThread user_thread;

    /* Padding for the stack canary */
    long _pad2[5];

    /* Kernel thread context */
    ucontext_t context;

} KernelThreadControlBlock;

/**
 * Kernel thread handle
 */
typedef KernelThreadControlBlock *KernelThread;

void kernel_threads_init(void);

void kernel_threads_deinit(void);

KernelThread kernel_thread_self(void);

#endif
