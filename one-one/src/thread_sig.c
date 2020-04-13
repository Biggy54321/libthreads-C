#include "./mods/sig.h"
#include "./thread.h"
#include "./thread_descr.h"

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

    /* Set the signal mask */
    sigprocmask(how, set, oldset);

    return THREAD_SUCCESS;
}

/**
 * @brief Delivers the specified signal to the target thread
 * @param[in] thread Thread handle for the target thread
 * @param[in] sig_num Signal number
 */
int thread_kill(Thread thread, int sig_num) {

    /* Check for errors */
    if (!thread) {

        /* Set the error number */
        thread_errno = EINVAL;
        return THREAD_FAIL;
    }
    if (td_is_exited(thread) ||
        td_is_joined(thread)) {

        /* Set the error number */
        thread_errno = EINVAL;
        return THREAD_FAIL;
    }

    /* Send the signal to the kernel thread */
    sig_send(td_get_ktid(thread), sig_num);

    return THREAD_SUCCESS;
}
