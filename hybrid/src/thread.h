#ifndef _THREAD_H_
#define _THREAD_H_

#include <errno.h>
#include <signal.h>

/**
 * Required enumerations
 */
enum {

    /* Thread function execution failed */
    THREAD_FAIL = -1,

    /* Thread function execution succeeded */
    THREAD_SUCCESS
};                              /* Thread return status */

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
typedef int ThreadOnce;
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
#define THREAD_ONCE_INIT (-1)

/**
 * Thread control routines
 */
int thread_create(Thread *thread, thread_start_t start, ptr_t arg, int type);
int thread_join(Thread thread, ptr_t *ret);
void thread_exit(ptr_t ret);
Thread thread_self(void);
int thread_yield(void);
int thread_equal(Thread thread1, Thread thread2);
int thread_once(ThreadOnce *once_control, void (*init_routine)(void));
ptr_t thread_main(ptr_t arg);

/**
 * Thread synchronization routines
 */
int thread_spin_init(ThreadSpinLock *spinlock);
int thread_spin_lock(ThreadSpinLock *spinlock);
int thread_spin_trylock(ThreadSpinLock *spinlock);
int thread_spin_unlock(ThreadSpinLock *spinlock);
int thread_spin_destroy(ThreadSpinLock *spinlock);

/**
 * Thread signal handling routines
 */
int thread_sigmask(int how, sigset_t *set, sigset_t *oldset);
int thread_kill(Thread Thread, int signo);

#endif
