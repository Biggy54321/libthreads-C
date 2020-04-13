#include <stddef.h>
#include <limits.h>
#include "./print.h"
#include "./print_ext.h"
#include <thread.h>

/**
 * User thread
 */
void *thread(void *arg) {

    /* Print information */
    debug_str("Inside thread()\n");

    return NULL;
}

/**
 * Main thread
 */
void *thread_main(void *arg) {

    Thread td;

    /* Print information */
    print_str("Thread creation testing\n\n");

    /* Test 1 */
    print_str("Test 1: Creating a thread with valid arguments\n");
    /* Create the thread */
    debug_str("thread_main() created thread()\n");
    if (thread_create(&td, thread, NULL) == THREAD_FAIL) {

        /* Print the information  */
        print_fail(1);
    } else {

        /* Join with thread */
        thread_join(td, NULL);
        /* Print the information  */
        print_succ(1);
    }

    newline;

    /* Test 2 */
    print_str("Test 2: Creating a thread with invalid thread descriptor\n");
    /* Create the thread */
    debug_str("thread_main() created a thread with thread object "
                "address as NULL\n");
    if (thread_create(NULL, thread, NULL) == THREAD_SUCCESS) {

        /* Print the information  */
        print_fail(2);
    } else {

        /* If the errno is is the expected one */
        if (thread_errno == EINVAL) {

            /* Print the information  */
            debug_str("thread_main() failed in creating the thread with "
                        "error number EINVAL\n");
            print_succ(2);
        } else {

            /* Print the information  */
            print_fail(2);
        }
    }

    newline;

    /* Test 3 */
    print_str("Test 3: Creating a thread with invalid start function\n");
    /* Create the thread */
    debug_str("thread_main() created a thread with start function address "
                "as NULL\n");
    if (thread_create(&td, NULL, NULL) == THREAD_SUCCESS) {

        /* Print the information  */
        print_fail(3);
    } else {

        /* If the errno is is the expected one */
        if (thread_errno == EINVAL) {

            /* Print the information  */
            debug_str("thread_main() failed in creating the thread with "
                      "error number EINVAL\n");
            print_succ(3);
        } else {

            /* Print the information  */
            print_fail(3);
        }
    }

    return NULL;
}
