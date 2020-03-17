#include <ucontext.h>

#include "./kernel_thread.h"
#include "./user_thread.h"
#include "./user_threads_list.h"
#include "./timer.h"
#include "./scheduler.h"

/**
 * @brief Yields the currently running user threads to pass the control to the
 *        scheduler
 * @param[in] ign Ignore
 */
static void _yield(int ign) {

    UserThread uthread;
    KernelThread kthread;

    /* Get the kernel thread handle */
    kthread = kernel_thread_self();

    /* Get the user thread handle */
    uthread = kthread->user_thread;

    /* Swap to the kernel context from the user context */
    swapcontext(&uthread->context, &kthread->context);
}

/**
 * @brief Kernel thread scheduler
 * @param[in] argument Ignore
 * @return Integer
 */
int scheduler(void *argument) {

    KernelThread kthread;
    UserThread uthread;
    Timer timer;

    /* Get the kernel thread handle */
    kthread = kernel_thread_self();

    /* Initialize the timer */
    timer_set(&timer, _yield, KERNEL_THREAD_TIME_SLICE_ms);

    /* For eternity */
    while (1) {

        /* Lock the user threads list */
        list_lock();

        if (list_is_empty()) {

            /* Unlock the user threads list */
            list_unlock();

            continue;
        }

        /* Get the user thread to be scheduled */
        uthread = list_get_thread();

        /* Unlock the user threads list */
        list_unlock();

        /* Set the mapping of the user thread to the current kernel thread */
        kthread->user_thread = uthread;

        /* Start the timer */
        timer_start(&timer);

        /* Set user context onto kernel context */
        swapcontext(&kthread->context, &uthread->context);

        /* Stop the timer */
        timer_stop(&timer);

        /* Reset the currently mapped user thread in the kernel thread */
        kthread->user_thread = NULL;

        /* Add the thread to the list only if it is not finished */
        if (uthread->wait_word == uthread->thread_id) {

            /* Lock the user threads list */
            list_lock();

            /* Add the task to the user threads list again */
            list_add_thread(uthread);

            /* Unlock the user threads list */
            list_unlock();
        }
    }
}

/**
 * Apparently we cannot change the #uc_link after the makecontext()
 */
