#define _GNU_SOURCE
#include <unistd.h>
#include <assert.h>
#include <stddef.h>
#include <signal.h>

#include <string.h>

#include "./sig.h"

/**
 * @brief Block all the signals
 *
 * Sets a mask so as to block all the signals
 */
void sig_block_all(void) {

    sigset_t mask;

    /* Set all the signals in the mask */
    sigfillset(&mask);

    /* Set the mask */
    sigprocmask(SIG_BLOCK, &mask, NULL);
}

/**
 * @brief Unblock all the signals
 *
 * Sets a mask so as to unblock all the signals
 */
void sig_unblock_all(void) {

    sigset_t mask;

    /* Set all the signals in the mask */
    sigfillset(&mask);

    /* Set the mask */
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

/**
 * @brief Send a signal to the given thread
 *
 * Sends the specified signal to the kernel thread represented by the given
 * thread id. It assumes that the caller thread and the target thread are in
 * in same thread group (which is common in threading libraries)
 *
 * @param[in] tid Thread id
 * @param[in] sig Signal number
 */
void sig_send(int tid, int sig) {

    /* Send the signal using kill system call */
    tgkill(getpid(), tid, sig);
}


/**
 * @brief Check for pending signals
 *
 * Checks if any previously blocked signals had been set and is now still
 * pending
 *
 * @return 0 if no signal is pending
 * @return 1 if atleast one signal is pending
 */
int sig_is_pending(void) {

    sigset_t mask;

    /* Initialize the mask to all zeros */
    sigemptyset(&mask);

    /* Get the pending signal set */
    sigpending(&mask);

    return !(sigisemptyset(&mask));
}
