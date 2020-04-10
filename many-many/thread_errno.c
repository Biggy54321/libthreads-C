#include "./thread.h"
#include "./thread_descr.h"
#include "./thread_errno.h"

/**
 * @brief Return the location of the error number variable for the calling
 *        thread
 * @return Integer address
 */
int *__get_thread_errno_loc(void) {

    Thread thread;

    /* Get the thread handle */
    thread = thread_self();

    /* Return the address of the local error number variable */
    return &thread->error;
}
