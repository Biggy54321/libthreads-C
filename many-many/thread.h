#ifndef _THREAD_H_
#define _THREAD_H_

#include <signal.h>

#include "./thread_errno.h"

/**
 * Required enumerations
 */
enum ThreadReturn {

    /* Successful execution */
    THREAD_SUCCESS,

    /* Failed execution */
    THREAD_FAIL
};

/**
 * Required structures
 */
struct Thread;
struct ThreadSpinLock;

/**
 * Required typedefs
 */
typedef enum ThreadType ThreadType;
typedef enum ThreadReturn ThreadReturn;
typedef struct Thread *Thread;
typedef struct ThreadSpinLock *ThreadSpinLock;
typedef void *ptr_t;
typedef void *(*thread_start_t)(void *);

/**
 * Required definitions
 */
#define thread_errno (*__get_thread_errno_loc())

/**
 * Thread control routines
 */
void thread_create(Thread *thread, thread_start_t start, ptr_t arg);
void thread_join(Thread thread, ptr_t *ret);
void thread_exit(ptr_t ret);
Thread thread_self(void);
void *thread_main(void *arg);

/**
 * Thread synchronization routines
 */
void thread_spin_init(ThreadSpinLock *spinlock);
void thread_spin_lock(ThreadSpinLock *spinlock);
void thread_spin_unlock(ThreadSpinLock *spinlock);
void thread_spin_destroy(ThreadSpinLock *spinlock);

/**
 * Thread signal handling routines
 */
void thread_sigmask(int how, sigset_t *set, sigset_t *oldset);
void thread_kill(Thread Thread, int signo);

#endif
