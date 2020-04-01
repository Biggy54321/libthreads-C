#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "./mods/utils.h"
#include "./mods/stack.h"
#include "./thread_cntl.h"

/* Clone flags for the one-one thread */
#define CLONE_FLAGS                                 \
    (CLONE_VM | CLONE_FS | CLONE_FILES |            \
     CLONE_SIGHAND | CLONE_THREAD |                 \
     CLONE_SYSVSEM | CLONE_PARENT_SETTID |          \
     CLONE_CHILD_CLEARTID | CLONE_SETTLS)           \
/* Long jump return value (should be non-zero) */
#define LONGJMP_RET_VAL (1u)

/**
 * @brief The thread start function which initiates the actual start
 *        function
 * @param[in] arg Not used
 * @return Integer
 */
int _thread_start(ptr_t arg) {

    Thread thread;

    /* Get the thread handle */
    thread = thread_self();

    /* Set the exit point */
    if (!setjmp(thread->exit_env)) {

        /* Set the initialization status */
        thread->is_init = 1;

        /* Launch the thread */
        thread->return_value = thread->start_routine(thread->argument);
    }

    return 0;
}

/**
 * @brief Creates a thread of execution
 * @param[out] thread Pointer to the thread handle
 * @param[in] start_routine Start function of the thread
 * @param[in] argument Pointer to the argument
 * @return Thread return status enumeration
 */
ThreadReturn thread_create(
        Thread *thread,
        thread_start_t start_routine,
        ptr_t argument) {

    /* Check for errors */
    if (!thread) {

        return THREAD_FAIL;
    }
    if (!start_routine) {

        return THREAD_FAIL;
    }

    /* Create the thread TLS */
    *thread = malloc(THREAD_CONTROL_BLOCK_SIZE);
    /* Check for errors */
    if (!(*thread)) {

        return THREAD_FAIL;
    }

    /* Set the start routine */
    (*thread)->start_routine = start_routine;

    /* Set the start routine argument */
    (*thread)->argument = argument;

    /* Allocate the stack */
    if (stack_alloc(&((*thread)->stack_base),
                    &((*thread)->stack_limit)) == -1) {

        return THREAD_FAIL;
    }

    /* Set the initialization status */
    (*thread)->is_init = 0;

    /* Create the kernel thread */
    (*thread)->thread_id = clone(_thread_start,
                                 (*thread)->stack_base + (*thread)->stack_limit,
                                 CLONE_FLAGS,
                                 NULL,
                                 &((*thread)->futex_word),
                                 *thread,
                                 &((*thread)->futex_word));
    /* Check for errors */
    if ((*thread)->thread_id == -1) {

        return THREAD_FAIL;
    }

    return THREAD_OK;
}

/**
 * @brief Waits for the specified target thread to stop
 * @param[in] thread Thread handle
 * @param[out] return_value Pointer to the return value
 */
ThreadReturn thread_join(
        Thread thread,
        ptr_t *return_value) {

    int ret_val;

    /* Check for errors */
    if (!thread) {

        return THREAD_FAIL;
    }

    /* Wait till the target thread is initialized */
    while (!thread->is_init);

    /* Wait on the target thread's futex word */
    ret_val = futex(&thread->futex_word, FUTEX_WAIT, thread->thread_id);

    /* Check for errors */
    if ((ret_val == -1) && (errno != EAGAIN)) {

        return THREAD_FAIL;
    }

    /* Free the stack */
    if (stack_free(&thread->stack_base, &thread->stack_limit) == -1) {

        return THREAD_FAIL;
    }

    /* If the user requested the return value */
    if (return_value) {

        *return_value = thread->return_value;
    }

    /* Free the thread control block */
    free(thread);

    return THREAD_OK;
}

/**
 * @brief Returns the thread handle of the current thread
 * @return Thread handle
 * @note The thread handle will be valid only if the thread is created using
 *       thread_create()
 */
Thread thread_self(void) {

    /* Rerad the FS register value */
    return (Thread)get_fs();
}

/**
 * @brief Terminates the calling thread
 * @param[in] return_value Return status of the thread
 */
void thread_exit(ptr_t return_value) {

    Thread thread;

    /* Get the thread handle */
    thread = thread_self();

    /* Set the return value */
    thread->return_value = return_value;

    /* Jump to the exit location */
    longjmp(thread->exit_env, LONGJMP_RET_VAL);
}
