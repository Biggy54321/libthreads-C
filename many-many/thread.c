#include <stdatomic.h>
#include <unistd.h>
#include <syscall.h>
#include <asm/prctl.h>
#include <sys/prctl.h>

#include "./init.h"
#include "./sched.h"
#include "./list.h"
#include "./lock.h"
#include "./stack.h"
#include "./thread.h"

/**
 * @brief Atomic compare and swap
 * @param[in] addr Address of lock variable
 * @param[in] old_val Old value expected in the lock variable
 * @param[in] new_value New value to be set in the lock variable
 * @return 1 if the value at addr is updated
 * @return 0 if the value at addr is not updated
 */
static inline int _atomic_cas(int *addr, int old_val, int new_val) {

    /* Use the atomic function to compare and exchange */
    return atomic_compare_exchange_strong(addr, &old_val, new_val);
}

/**
 * @brief Get FS register value
 * @return Value of the FS register (long)
 */
static inline long _get_fs(void) {

    long addr;

    /* Use the syscall wrapper */
    syscall(SYS_arch_prctl, ARCH_GET_FS, &addr);

    return addr;
}

/**
 * @brief Initialize the thread library
 */
void thread_lib_init(void) {

    /* Initialize the schedulers */
    sched_init();
}

/**
 * @brief Creates a user thread
 * @param[out] thread Pointer to the thread handle
 * @param[in] start_routine Start routine of the thread
 * @param[in] argument Argument to the start routine
 */
void thread_create(
        Thread *thread,
        void *(*start_routine)(void *),
        void *argument) {

    /* Create the user thread */
    init_thread(thread, start_routine, argument);

    /* Add the thread to the list of threads */
    list_enqueue(*thread);
}

/**
 * @brief Waits for the target threads completion
 * @param[in] thread Thread handle
 * @param[out] return_value Pointer to the return value
 */
void thread_join(Thread thread, void **return_value) {

    /* Wait for target thread to change its state */
    while (!_atomic_cas((int *)&thread->state, THREAD_INACTIVE, THREAD_JOINED));

    /* If the user requested the return value */
    if (return_value) {

        /* Get the return value */
        *return_value = thread->return_value;
    }

    /* Free the thread stack */
    stack_free(&(thread->curr_context.uc_stack));

    /* Dont free the thread control block as other threads may be waiting */
}

/**
 * @brief Exits from the current user thread
 * @param[in] return_value Return value of the current thread
 * @note May give undefined result if used in threads not created using
 *       thread_create()
 */
void thread_exit(void *return_value) {

    Thread thread;

    /* Get the current thread handle */
    thread = (Thread)_get_fs();

    /* Set the return value */
    thread->return_value = return_value;

    /* Set the state of the thread as dead */
    _atomic_cas((int *)&thread->state, THREAD_ACTIVE, THREAD_INACTIVE);

    /* Set the context of the scheduler */
    setcontext(&thread->ret_context);
}

/**
 * @brief Initializes the lock
 * @param[in] lock Pointer to the lock instance
 */
void thread_lock_init(Lock *lock) {

    /* Check for errors */
    assert(lock);

    /* Set the lock to not acquired status */
    *lock = LOCK_NOT_ACQUIRED;
}

/**
 * @brief Acquires the lock
 * @param[in] lock Pointer to the lock instance
 */
void thread_lock(Lock *lock) {

    /* Acquire the lock */
    lock_acquire(lock);
}

/**
 * @brief Releases the lock
 * @param[in] lock Pointer to the lock instance
 */
void thread_unlock(Lock *lock) {

    /* Release the lock */
    lock_release(lock);
}

/**
 * @brief Delivers the signal to the thread
 * @param[in] thread Thread handle
 * @param[in] sig_num Signal to be delivered
 */
void thread_kill(Thread thread, int sig_num) {

}

/**
 * @brief Returns the thread handle
 * @return Thread handle instance
 */
Thread thread_self(void) {

    /* Return the value of the fs register */
    return (Thread)_get_fs();
}

/**
 * @brief Deinitialize the thread library
 */
void thread_lib_deinit(void) {

    /* Deinitialize the schedulers */
    sched_deinit();
}


/* TODO
 * 1. IMPLEMENT THE THREAD KILL
 * 2. IMPROVE ERROR CHECKING
 */
