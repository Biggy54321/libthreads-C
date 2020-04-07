#define _GNU_SOURCE
#include <unistd.h>
#include <assert.h>
#include <stddef.h>
#include <signal.h>

#include "./sig.h"

/**
 * @brief Block all the signals
 *
 * Sets a mask so as to block all the signals
 *
 * @return 0 if success
 * @return -1 if failure
 */
int sig_block_all(void) {

    sigset_t mask;

    /* Set all the signals in the mask */
    if (sigfillset(&mask) == -1) {

        return -1;
    }

    /* Set the mask */
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {

        return -1;
    }

    return 0;
}

/**
 * @brief Unblock all the signals
 *
 * Sets a mask so as to unblock all the signals
 *
 * @return 0 if success
 * @return -1 if failure
 */
int sig_unblock_all(void) {

    sigset_t mask;

    /* Set all the signals in the mask */
    if (sigfillset(&mask) == -1) {

        return -1;
    }

    /* Set the mask */
    if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1) {

        return -1;
    }

    return 0;
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
 * @return 0 if success
 * @return -1 if failure
 */
int sig_send(int tid, int sig) {

    /* Send the signal using kill system call */
    if (tgkill(getpid(), tid, sig) == -1) {

        return -1;
    }

    return 0;
}


/**
 * @brief Check for pending signals
 *
 * Checks if any previously blocked signals had been set and is now still
 * pending
 *
 * @return 0 if no signal is pending
 * @return 1 if atleast one signal is pending
 * @return -1 if failure
 */
int sig_is_pending(void) {

    sigset_t mask;

    /* Initialize the mask to all zeros */
    if (sigemptyset(&mask) == -1) {

        return -1;
    }

    /* Get the pending signal set */
    if (sigpending(&mask) == -1) {

        return -1;
    }

    return !(sigisemptyset(&mask));
}
