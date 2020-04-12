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
int thread_sigmask(int how, sigset_t *set, sigset_t *oldset) {

    Thread thread;

    /* Check for errors */
    if ((!set) ||               /* Pointer to new set is not valid */
        (!oldset)) {            /* Pointer to the old set is not valid */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Get the thread handle */
    thread = thread_self();

    /* Remove SIGALRM from the signal set (just becuz i use it) */
    sigdelset(set, SIGALRM);

    /* Disable the interrupts */
    td_disable_intr(thread);

    /* Call the signal process mask function */
    sigprocmask(how, set, oldset);

    /* Enable the interrupts */
    td_enable_intr(thread);

    return THREAD_SUCCESS;
}

/**
 * @brief Kill a thread
 *
 * Send a signal to the target thread
 *
 * @param[in] thread Target thread handle
 * @param[in] signo Signal number
 */
int thread_kill(Thread thread, int signo) {

    /* Check for errors */
    if ((!thread) ||            /* Thread descriptor is invalid */
        (signo < _SIG_LOW) ||   /* Signal number is below range */
        (signo > _SIG_HIGH)) {  /* Signal number is above range */

        /* Set the errno */
        thread_errno = EINVAL;
        /* Return failure */
        return THREAD_FAIL;
    }

    /* Acquire the member lock */
    td_lock(thread);

    /* Add the requested signal to the pending signal bitmask */
    td_set_sig_pending(thread, signo);

    /* Release the member lock */
    td_unlock(thread);

    return THREAD_SUCCESS;
}
