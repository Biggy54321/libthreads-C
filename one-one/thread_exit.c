#include "./thread_exit.h"

/**
 * @brief Terminates the calling thread
 * @param[in] return_value Return status of the thread
 */
void thread_exit(ptr_t return_value) {

    Thread thread;

    /* Get the thread handle for the calling thread */
    thread = thread_self();

    /* Set the return value of the thread */
    thread->return_value = return_value;

    /* Jump to the exit location */
    longjmp(thread->exit_env, 1);
}
