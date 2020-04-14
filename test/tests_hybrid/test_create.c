#include <stddef.h>
#include "./print.h"
#include "./print_ext.h"
#include <thread.h>

/**
 * One one mapped user thread
 */
void *thread_one(void *arg) {

    /* Print information */
    debug_str("Inside thread_one()\n");

    return NULL;
}

/**
 * Many many mapped user thread
 */
void *thread_many(void *arg) {

    /* Print information */
    debug_str("Inside thread_many()\n");

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
    print_str("Test 1: Create a one one thread\n");
    debug_str("thread_main() created thread_one()\n");
    if (thread_create(&td, thread_one, NULL, THREAD_TYPE_ONE_ONE)
        == THREAD_SUCCESS) {

        /* Join with the thread */
        debug_str("thread_main() called join on thread_one()\n");
        thread_join(td, NULL);
        print_succ(1);

    } else {

        print_fail(1);
    }

    newline;

    /* Test 2 */
    print_str("Test 2: Create a many many thread\n");
    debug_str("thread_main() created thread_many()\n");
    if (thread_create(&td, thread_many, NULL, THREAD_TYPE_MANY_MANY)
        == THREAD_SUCCESS) {

        /* Join with the thread */
        debug_str("thread_main() called join on thread_many()\n");
        thread_join(td, NULL);
        print_succ(2);

    } else {

        print_fail(2);
    }

    newline;

    /* Test 3 */
    print_str("Test 3: Creating a thread with invalid thread descriptor\n");
    /* Create the thread */
    debug_str("thread_main() created a thread with thread object "
              "address as NULL\n");
    if (thread_create(NULL, thread_one, NULL, THREAD_TYPE_ONE_ONE)
        == THREAD_SUCCESS) {

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

    newline;

    /* Test 4 */
    print_str("Test 4: Creating a thread with invalid start function\n");
    /* Create the thread */
    debug_str("thread_main() created a thread with start function address "
              "as NULL\n");
    if (thread_create(&td, NULL, NULL, THREAD_TYPE_MANY_MANY)
        == THREAD_SUCCESS) {

        /* Print the information  */
        print_fail(4);
    } else {

        /* If the errno is is the expected one */
        if (thread_errno == EINVAL) {

            /* Print the information  */
            debug_str("thread_main() failed in creating the thread with "
                      "error number EINVAL\n");
            print_succ(4);
        } else {

            /* Print the information  */
            print_fail(4);
        }
    }

    newline;

    /* Test 5 */
    print_str("Test 5: Creating a thread with invalid mapping type\n");
    /* Create the thread */
    debug_str("thread_main() created a thread with mapping type argument "
              "being neither THREAD_TYPE_ONE_ONE nor THREAD_TYPE_MANY_MANY\n");
    if (thread_create(&td, thread_many, NULL, 4) == THREAD_SUCCESS) {

        /* Print the information  */
        print_fail(5);
    } else {

        /* If the errno is is the expected one */
        if (thread_errno == EINVAL) {

            /* Print the information  */
            debug_str("thread_main() failed in creating the thread with "
                      "error number EINVAL\n");
            print_succ(5);
        } else {

            /* Print the information  */
            print_fail(5);
        }
    }


    return NULL;
}
