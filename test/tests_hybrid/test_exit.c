#include <stddef.h>
#include "./print.h"
#include "./print_ext.h"
#include <thread.h>

/* Return status of the two threads  */
#define RET_THREAD_RET_STATUS  (123u)
#define EXIT_THREAD_RET_STATUS (456u)

/**
 * User thread using a return
 */
void *ret_thread(void *arg) {

    /* Print information */
    debug_str("In ret_thread(), returning with value ");
    debug_int(RET_THREAD_RET_STATUS);
    debug_newline;

    return (void *)RET_THREAD_RET_STATUS;
}

/**
 * Exit user thread subfunction
 */
void exit_thread_sub(void) {

    /* Print information */
    debug_str("In exit_thread_sub(), exiting with value ");
    debug_int(EXIT_THREAD_RET_STATUS);
    debug_newline;

    /* Exit */
    thread_exit((void *)EXIT_THREAD_RET_STATUS);
}

/**
 * User thread using a exit from a sub function
 */
void *exit_thread(void *arg) {

    /* Print information */
    debug_str("In exit_thread(), calling exit_thread_sub()\n");

    /* Call the subfunction */
    exit_thread_sub();

    return (void *)0;
}

/**
 * Main thread
 */
void *thread_main(void *main_arg) {

    Thread td;
    void *ret;

    /* Print information */
    print_str("Thread exit testing\n\n");

    /* Test 1 */
    print_str("Test 1: One one thread returning from the start function\n");
    /* Create the thread */
    debug_str("thread_main() created ret_thread() as a one one thread\n");
    thread_create(&td, ret_thread, NULL, THREAD_TYPE_ONE_ONE);
    /* Join with the thread to get the return status */
    debug_str("thread_main() called join on ret_thread()\n");
    thread_join(td, &ret);
    /* Check the return value */
    if ((int)ret == RET_THREAD_RET_STATUS) {

        /* Print the information */
        debug_str("thread_main() received a return value of ");
        debug_int(RET_THREAD_RET_STATUS);
        debug_newline;
        print_succ(1);
    } else {

        /* Print the information */
        print_fail(1);
    }

    newline;

    /* Test 2 */
    print_str("Test 2: One one thread exiting from some subfunction\n");
    /* Create the thread */
    debug_str("thread_main() created exit_thread() as a one one thread\n");
    thread_create(&td, exit_thread, NULL, THREAD_TYPE_ONE_ONE);
    /* Join with the thread to get the return status */
    debug_str("thread_main() called join on exit_thread()\n");
    thread_join(td, &ret);
    /* Check the return value */
    if ((int)ret == EXIT_THREAD_RET_STATUS) {

        /* Print the information */
        debug_str("thread_main() received a return value of ");
        debug_int(EXIT_THREAD_RET_STATUS);
        debug_newline;
        print_succ(2);
    } else {

        /* Print the information */
        print_fail(2);
    }

    newline;

    /* Test 3 */
    print_str("Test 3: Many many thread returning from the start function\n");
    /* Create the thread */
    debug_str("thread_main() created ret_thread() as a many many thread\n");
    thread_create(&td, ret_thread, NULL, THREAD_TYPE_MANY_MANY);
    /* Join with the thread to get the return status */
    debug_str("thread_main() called join on ret_thread()\n");
    thread_join(td, &ret);
    /* Check the return value */
    if ((int)ret == RET_THREAD_RET_STATUS) {

        /* Print the information */
        debug_str("thread_main() received a return value of ");
        debug_int(RET_THREAD_RET_STATUS);
        debug_newline;
        print_succ(3);
    } else {

        /* Print the information */
        print_fail(3);
    }

    newline;

    /* Test 2 */
    print_str("Test 4: Many many thread exiting from some subfunction\n");
    /* Create the thread */
    debug_str("thread_main() created exit_thread() as a many many thread\n");
    thread_create(&td, exit_thread, NULL, THREAD_TYPE_MANY_MANY);
    /* Join with the thread to get the return status */
    debug_str("thread_main() called join on exit_thread()\n");
    thread_join(td, &ret);
    /* Check the return value */
    if ((int)ret == EXIT_THREAD_RET_STATUS) {

        /* Print the information */
        debug_str("thread_main() received a return value of ");
        debug_int(EXIT_THREAD_RET_STATUS);
        debug_newline;
        print_succ(2);
    } else {

        /* Print the information */
        print_fail(2);
    }

    return NULL;
}
