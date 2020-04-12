#ifndef _THREAD_H_
#define _THREAD_H_

#include <errno.h>
#include <signal.h>

/**
 * Required enumerations
 */
enum {

    /* One-one thread type */
    THREAD_TYPE_ONE_ONE,

    /* Many-many thread type */
    THREAD_TYPE_MANY_MANY
};                              /* Thread mapping type */

/**
 * Required structures
 */
struct Thread;
struct ThreadSpinLock;

/**
 * Required typedefs
 */
typedef struct Thread *Thread;
typedef struct ThreadSpinLock *ThreadSpinLock;
typedef void *ptr_t;
typedef void *(*thread_start_t)(void *);

/**
 * Get the location of the error variable
 */
int *__get_thread_errno_loc(void);

/**
 * Required definitions
 */
#define thread_errno (*__get_thread_errno_loc())

/**
 * Thread control routines
 */
void thread_create(Thread *thread, thread_start_t start, ptr_t arg, int type);
void thread_join(Thread thread, ptr_t *ret);
void thread_exit(ptr_t ret);
Thread thread_self(void);
void thread_yield(void);
ptr_t thread_main(ptr_t arg);

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
