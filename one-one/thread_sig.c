#include <unistd.h>

#include "./mods/utils.h"
#include "./thread_sig.h"

/**
 * @brief Update the signal mask of the current thread
 * @param[in] how What action to perform
 * @param[in] set Pointer to the signal set to be worked upon
 * @param[out] oldset Pointer to the signal set which will store the old signal
 *                    mask. Will store the previously blocked signal set
 */
ThreadReturn thread_sigmask(int how, sigset_t *set, sigset_t *oldset) {

    /* Check for errors */
    if (!set) {

        return THREAD_FAIL;
    }
    if (!oldset) {

        return THREAD_FAIL;
    }

    /* Set the signal mask */
    if (sigprocmask(how, set, oldset) == -1) {

        return THREAD_FAIL;
    }

    return THREAD_OK;
}

/**
 * @brief Delivers the specified signal to the target thread
 * @param[in] thread Thread handle for the target thread
 * @param[in] sig_num Signal number
 * @return Thread return status
 */
ThreadReturn thread_kill(Thread thread, int sig_num) {

    int tgid;

    /* Check for errors */
    if (!thread) {

        return THREAD_FAIL;
    }

    /* Wait till the target thread is initialized */
    while (!thread->is_init);

    /* Get the thread group id */
    tgid = getpid();

    /* Send the signal to the kernel thread */
    if (tgkill(tgid, thread->thread_id, sig_num) == -1) {

        return THREAD_FAIL;
    }

    return THREAD_OK;
}
