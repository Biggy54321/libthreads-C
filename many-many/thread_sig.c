#include "./mods/lock.h"
#include "./mods/sig.h"
#include "./thread.h"
#include "./thread_descr.h"

/* Lowest signal number */
#define _SIG_LOW   (1u)
/* Highest signal number */
#define _SIG_HIGH  (31u)
/* Get the signal mask */
#define _SIG_GET_MASK(signo) (1 << ((signo) - 1))

/**
 * @brief Update the signal mask
 *
 * Changes the current signal mask of thread
 *
 * @param[in] how What action to perform
 * @param[in] set Pointer to the signal set to be worked upon
 * @param[out] oldset Pointer to the signal set which will store the old signal
 *                    mask. Will store the previously block signal set
 */
void thread_sigmask(int how, sigset_t *set, sigset_t *oldset) {

    /* Check for errors */
    assert(set);
    assert(oldset);

    /* Remove SIGALRM from the signal set (just becuz i use it) */
    sigdelset(set, SIGALRM);

    /* Call the signal process mask function */
    sigprocmask(how, set, oldset);
}

/**
 * @brief Kill a thread
 *
 * Send a signal to the target thread
 *
 * @param[in] thread Target thread handle
 * @param[in] signo Signal number
 */
void thread_kill(Thread thread, int signo) {

    /* Check for errors */
    assert(thread);
    assert((signo >= _SIG_LOW) && (signo <= _SIG_HIGH));

    /* Acquire the member lock */
    lock_acquire(&thread->mem_lock);

    /* Add the requested signal to the pending signal bitmask */
    thread->pend_sig |= _SIG_GET_MASK(signo);

    /* Release the member lock */
    lock_release(&thread->mem_lock);
}
