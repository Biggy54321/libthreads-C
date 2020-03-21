#ifndef _HTHREAD_DEFS_H_
#define _HTHREAD_DEFS_H_

#include <ucontext.h>
#include "./lib/list.h"
#include "./lib/lock.h"

/**
 * Thread mapping type
 */
typedef enum _HThreadType {

    /* Thread mapping type is one one */
    HTHREAD_TYPE_ONE_ONE,

    /* Thread mapping type is many many */
    HTHREAD_TYPE_MANY_MANY
} HThreadType;

/**
 * Thread state
 */
typedef enum _HThreadState {

    /* Thread is in initialization phase i.e. not runnable */
    HTHREAD_STATE_INIT,

    /* Thread is active i.e. runnable */
    HTHREAD_STATE_ACTIVE,

    /* Thread is inactive i.e. not runnable*/
    HTHREAD_STATE_INACTIVE,

    /* Thread is joined i.e. it is not joinable */
    HTHREAD_STATE_JOINED,
} HThreadState;

/**
 * Base thread control block members
 */
#define HTHREAD_BASE_MEMBERS                    \
    /* Thread id */                             \
    int id;                                     \
                                                \
    /* Thread mapping type */                   \
    HThreadType type;                           \
                                                \
    /* Thread state */                          \
    HThreadState state;                         \
                                                \
    /* Thread starting function */              \
    void *(*start)(void *);                     \
                                                \
    /* Thread starting function argument */     \
    void *arg;                                  \
                                                \
    /* Thread starting function return value */ \
    void *ret;                                  \
                                                \
    /* Padding for stack canary */              \
    long _pad;                                  \
                                                \
    /* Wait word */                             \
    int wait;

/**
 * General thread control block structure
 */
struct _HThread {

    /* Include the base members of the thread control block */
    HTHREAD_BASE_MEMBERS;
};

/**
 * Many-many thread control block structure
 */
struct _HThreadManyMany {

    /* Include the base members of the thread control block */
    HTHREAD_BASE_MEMBERS;

    /* Current context of the thread */
    ucontext_t *curr_cxt;

    /* Return context of the scheduler */
    ucontext_t *ret_cxt;

    /* Linked list links */
    ListMember list_mem;
};

/**
 * One-one thread control block structure
 */
struct _HThreadOneOne {

    /* Include all the base members of the thread */
    HTHREAD_BASE_MEMBERS;

    /* Kernel thread id */
    int tid;

    /* Thread stack */
    stack_t stack;

    /* Return context */
    ucontext_t *ret_cxt;
};

/* One one thread local storage size */
#define HTHREAD_ONE_ONE_TLS_SIZE   (sizeof(struct _HThreadOneOne))
/* Many many thread local storage size */
#define HTHREAD_MANY_MANY_TLS_SIZE (sizeof(struct _HThreadManyMany))

/* Upcast the general thread handle to one one thread handle */
#define ONE_ONE(hthread)   ((struct _HThreadOneOne *)(hthread))
/* Upcast the general thread handle to many many thread handle */
#define MANY_MANY(hthread) ((struct _HThreadManyMany *)(hthread))
/* Downcast the any specific handle to general handle */
#define BASE(thread)        ((struct _HThread *)thread)

/**
 * Thread handle for the application program
 */
typedef struct _HThread *HThread;

/**
 * Thread mutex lock
 */
typedef struct _HThreadMutex {

    /* Owner thread of the lock */
    HThread owner;

    /* Lock word */
    Lock lock;
} HThreadMutex;

#endif
