#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <syscall.h>

#include "./thread_kill.h"

/**
 * @brief Sends a signal to a thread in the thread group
 * @param[in] tgid Thread group id
 * @param[in] tid Thread id
 * @param[in] sig Signal number
 * @return Integer status
 */
int _tgkill(int tgid, int tid, int sig) {

    /* Use the system call wrapper to invoke the call */
    return syscall(SYS_tgkill, tgid, tid, sig);
}

/**
 * @brief Delivers the specified signal to the target thread
 * @param[in] thread Thread handle for the target thread
 * @param[in] sig_num Signal number
 * @return Thread return status
 */
ThreadReturn thread_kill(Thread thread, int sig_num) {

    uint32_t thread_group_id;

    /* Get the thread group id */
    thread_group_id = getpid();

    /* Send the kill signal to the target thread */
    if (_tgkill(thread_group_id, thread->thread_id, sig_num) == -1) {

        /* Return error status */
        return THREAD_FAIL;
    }

    /* Return success status */
    return THREAD_OK;
}
