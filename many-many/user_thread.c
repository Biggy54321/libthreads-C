#include <stdlib.h>

#include "./user_thread.h"
#include "./kernel_thread.h"
#include "./stack.h"
#include "./spin_lock.h"

/* Next thread id to be assigned to a user thread */
static int _next_user_thread_id = 1;
/* Lock for the next thread id */
static Lock _lock = SPIN_LOCK_NOT_TAKEN;

/**
 * @brief Actual user thread start function
 */
static void _user_thread_start(void) {

    UserThread uthread;
    KernelThread kthread;

    /* Get the kernel thread */
    kthread = kernel_thread_self();

    /* Get the user thread */
    uthread = kthread->user_thread;

    /* Set the exit jump location */
    if (!setjmp(uthread->exit_env)) {

        /* Call the user thread start routine */
        uthread->return_value = uthread->start_routine(uthread->argument);
    }

    /* Set the wait word to zero */
    uthread->wait_word = 0;

    setcontext(&kthread->context);
}

/**
 * @brief Allocate a user thread
 * @param[in] uthread Pointer to the user thread instance
 */
void user_thread_create(
        UserThread *uthread,
        void *(*start_routine)(void *),
        void *argument) {

    UserThreadControlBlock *utcb;

    /* Allocate the user thread */
    utcb = (UserThreadControlBlock *)malloc(sizeof(UserThreadControlBlock));

    /* Set the user thread address */
    *uthread = (UserThread)utcb;

    /* Acquire the lock for the thread id */
    spin_lock(&_lock);

    /* Set the thread id */
    utcb->thread_id = _next_user_thread_id++;

    /* Release the lock for the thread id */
    spin_unlock(&_lock);

    /* Set the start routine */
    utcb->start_routine = start_routine;

    /* Set the start routine arguments */
    utcb->argument = argument;

    /* Initialize the wait word */
    utcb->wait_word = utcb->thread_id;

    /* Get the current context */
    getcontext(&utcb->context);

    /* Set the stack for the user thread */
    stack_alloc(&(utcb->context.uc_stack));

    /* Set the backlink of the user context */
    utcb->context.uc_link = NULL;

    /* Make the required context */
    makecontext(&utcb->context, _user_thread_start, 0);
}

/**
 * @brief Deallocate a user thread
 * @param[in] uthread Pointer to the user thread instance
 */
void user_thread_destroy(UserThread *uthread) {

    ;
}
