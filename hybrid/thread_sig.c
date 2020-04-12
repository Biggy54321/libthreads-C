#include "./mods/sig.h"
#include "./thread.h"
#include "./thread_descr.h"

/* Lowest signal number */
#define _SIG_LOW   (1u)
/* Highest signal number */
#define _SIG_HIGH  (31u)

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

    Thread thread;

    /* Check for errors */
    assert(set);
    assert(oldset);

    /* Get the thread handle */
    thread = thread_self();

    /* Remove SIGALRM from the signal set (just becuz i use it) */
    sigdelset(set, SIGALRM);

    /* Disable interrupts if many many thread */
    if (td_is_many_many(thread)) {

        td_mm_disable_intr(thread);
    }

    /* Call the signal process mask function */
    sigprocmask(how, set, oldset);

    /* Enable interrupts if many many thread */
    if (td_is_many_many(thread)) {

        td_mm_enable_intr(thread);
    }
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

    /* If the thread is one one */
    if (td_is_one_one(thread)) {

        /* Send the signal to the target thread */
        sig_send(td_oo_get_ktid(thread), signo);
    } else {

        /* Acquire the member lock */
        td_lock(thread);

        /* Set the signal as pending */
        td_mm_set_sig_pending(thread, signo);

        /* Release the member lock */
        td_unlock(thread);
    }
}
