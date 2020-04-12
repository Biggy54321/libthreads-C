#ifndef _THREAD_H_
#define _THREAD_H_

#include <errno.h>
#include <signal.h>

/**
 * Required enumerations
 */
enum {

    /* Failed execution */
    THREAD_FAIL = -1,

    /* Successful execution */
    THREAD_SUCCESS
};                              /* Thread return status */

/**
 * Required structures
 */
struct Thread;
struct ThreadSpinLock;
struct ThreadMutex;

/**
 * Required typedefs
 */
typedef enum ThreadType ThreadType;
typedef enum ThreadReturn ThreadReturn;
typedef struct Thread *Thread;
typedef struct ThreadSpinLock *ThreadSpinLock;
typedef struct ThreadMutex *ThreadMutex;
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
int thread_create(Thread *thread, thread_start_t start, ptr_t arg);
int thread_join(Thread thread, ptr_t *ret);
void thread_exit(ptr_t ret);
Thread thread_self(void);
int thread_yield(void);
void *thread_main(void *arg);

/**
 * Thread synchronization routines
 */
int thread_spin_init(ThreadSpinLock *spinlock);
int thread_spin_lock(ThreadSpinLock *spinlock);
int thread_spin_unlock(ThreadSpinLock *spinlock);
int thread_spin_destroy(ThreadSpinLock *spinlock);
int thread_mutex_init(ThreadMutex *mutex);
int thread_mutex_lock(ThreadMutex *mutex);
int thread_mutex_unlock(ThreadMutex *mutex);
int thread_mutex_destroy(ThreadMutex *mutex);

/**
 * Thread signal handling routines
 */
int thread_sigmask(int how, sigset_t *set, sigset_t *oldset);
int thread_kill(Thread Thread, int signo);

#endif

/**
 * Check if races are occuring if the thread library functions are interrupted
 * by signals, which call the thread library functions in them
 */
