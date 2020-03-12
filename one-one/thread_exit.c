#include "./thread_exit.h"

/* Return value of the long jump (must be any non zero integer) */
#define LONGJMP_RET_VAL (1)

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
    longjmp(thread->exit_env, LONGJMP_RET_VAL);
}
