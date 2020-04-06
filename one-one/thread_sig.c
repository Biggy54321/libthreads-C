#include <unistd.h>

#include "./mods/utils.h"
#include "./thread_sig.h"
#include "./thread_errno.h"

/**
 * @brief Update the signal mask of the current thread
 * @param[in] how What action to perform
 * @param[in] set Pointer to the signal set to be worked upon
 * @param[out] oldset Pointer to the signal set which will store the old signal
 *                    mask. Will store the previously blocked signal set
 * @return 0 if success
 * @return -1 if failure, sets the errno
 */
int thread_sigmask(int how, sigset_t *set, sigset_t *oldset) {

    /* Check for errors */
    if (!set) {

        THREAD_RET_FAIL(EINVAL);
    }
    if (!oldset) {

        THREAD_RET_FAIL(EINVAL);
    }

    /* Set the signal mask */
    if (sigprocmask(how, set, oldset) == -1) {

        return THREAD_FAIL;
    }

    return THREAD_SUCCESS;
}

/**
 * @brief Delivers the specified signal to the target thread
 * @param[in] thread Thread handle for the target thread
 * @param[in] sig_num Signal number
 * @return 0 if success
 * @return -1 if failure, sets the errno
 */
int thread_kill(Thread thread, int sig_num) {

    int tgid;

    /* Check for errors */
    if (!thread) {

        THREAD_RET_FAIL(EINVAL);
    }
    if (thread->thread_state == THREAD_STATE_EXITED) {

        THREAD_RET_FAIL(EINVAL);
    }
    if (thread->thread_state == THREAD_STATE_JOINED) {

        THREAD_RET_FAIL(EINVAL);
    }

    /* Get the thread group id */
    tgid = getpid();

    /* Send the signal to the kernel thread */
    if (tgkill(tgid, thread->thread_id, sig_num) == -1) {

        return THREAD_FAIL;
    }

    return THREAD_SUCCESS;
}
