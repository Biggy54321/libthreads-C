#ifndef _HTHREAD_PUB_H_
#define _HTHREAD_PUB_H_

/**
 * Thread type
 */
typedef enum HThreadType {

    /* Thread mapping type is one one */
    HTHREAD_TYPE_ONE_ONE,

    /* Thread mapping type is many many */
    HTHREAD_TYPE_MANY_MANY

} HThreadType;

/**
 * Thread state
 */
typedef enum HThreadState {

    /* Thread is in initialization phase i.e. not runnable */
    HTHREAD_STATE_INIT,

    /* Thread is active i.e. runnable */
    HTHREAD_STATE_ACTIVE,

    /* Thread is waiting infinitely */
    HTHREAD_STATE_WAIT,

    /* Thread is inactive i.e. not runnable*/
    HTHREAD_STATE_INACTIVE,

    /* Thread is joined i.e. it is not joinable */
    HTHREAD_STATE_JOINED,

} HThreadState;

/**
 * Base TLS members
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
    /* Start routine */                         \
    void *(*start)(void *);                     \
                                                \
    /* Start routine argument */                \
    void *arg;                                  \
                                                \
    /* Start routine return value */            \
    void *ret;                                  \
                                                \
    /* Padding for stack canary */              \
    long _pad;                                  \
                                                \
    /* Wait word */                             \
    int wait;

/**
 * General TLS
 */
struct HThread {

    /* Include the base members of the thread control block */
    HTHREAD_BASE_MEMBERS;

};

/**
 * Thread handle
 */
typedef struct HThread *HThread;

#endif
