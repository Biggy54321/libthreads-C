#include <stddef.h>
#include "./print.h"
#include "./print_ext.h"
#include <thread.h>

/**
 * Macro to introduce some dummy work
 */
#define DELAY()                                 \
    {                                           \
        for (int i = 0; i < 100000; i++) {      \
                                                \
            /* Do some work */                  \
            i++;                                \
            i++;                                \
            i--;                                \
            i--;                                \
        }                                       \
    }

/**
 * User thread 1
 */
void *thread1(void *arg) {

    /* Add some delay */
    DELAY();

    /* Print information */
    debug_str("In thread1()\n");

    return NULL;
}

/**
 * User thread 2
 */
void *thread2(void *arg) {

    Thread *td1;

    /* Get the thread descriptor from the argument */
    td1 = arg;

    /* Add some delay */
    DELAY();

    /* Print information */
    debug_str("thread2() called join on thread1()\n");

    /* Join with the thread */
    if (thread_join(*td1, NULL) == THREAD_SUCCESS) {

        /* Print the information */
        print_fail(4);
    } else {

        /* Check the errno */
        if (thread_errno == EINVAL) {

            /* Print the information */
            debug_str("thread2() failed to join with thread1(), with error "
                      "number EINVAL\n");
            print_succ(4);
        } else {

            /* Print the information */
            print_fail(4);
        }
    }

    return NULL;
}

/**
 * User thread 3
 */
void *thread3(void *arg) {

    Thread *td;

    /* Get the thread descriptor */
    td = arg;

    /* Add some delay */
    DELAY();

    /* Print information */
    debug_str("thread3() called join on thread_main()\n");

    /* Call join with the thread */
    if (thread_join(*td, NULL) == THREAD_SUCCESS) {

        /* Print the information */
        print_fail(6);
    } else {

        /* Check the errno */
        if (thread_errno == EDEADLK) {

            /* Print the information */
            debug_str("thread3() failed to join with thread_main(), with "
                      "error number EDEADLK\n");
            print_succ(6);
        } else {

            /* Print the information */
            print_fail(6);
        }
    }

    return NULL;
}

/**
 * Main thread
 */
void *thread_main(void *main_arg) {

    Thread td1, td2, td3, td_me;

    /* Print information */
    print_str("Thread join testing\n\n");

    /* Get the thread object */
    td_me = thread_self();

    /* Test 1 */
    print_str("Test 1: Join with valid arguments and no deadlock scenarios\n");
    /* Create the thread */
    debug_str("thread_main() created thread1() as one one thread\n");
    thread_create(&td1, thread1, NULL, THREAD_TYPE_ONE_ONE);
    /* Join with the thread */
    debug_str("thread_main() called join on thread1()\n");
    if (thread_join(td1, NULL) == THREAD_SUCCESS) {

        /* Print the information */
        debug_str("thread_main() joined successfully\n");
        print_succ(1);
    } else {

        /* Print the information */
        print_fail(1);
    }

    newline;

    /* Test 2 */
    print_str("Test 2: Join with invalid thread descriptor\n");
    /* Create the thread */
    debug_str("thread_main() created thread1() as many many thread\n");
    thread_create(&td1, thread1, NULL, THREAD_TYPE_MANY_MANY);
    /* Join with thread */
    debug_str("thread_main() called join on NULL thread\n");
    if (thread_join(NULL, NULL) == THREAD_SUCCESS) {

        /* Print the information */
        print_fail(2);
    } else {

        /* Check the errno */
        if (thread_errno == EINVAL) {

            /* Print the information */
            debug_str("thread_main() failed to join, with "
                      "error number EINVAL\n");
            print_succ(2);
        } else {

            /* Print the information */
            print_fail(2);
        }
    }
    /* Join with the created thread */
    thread_join(td1, NULL);

    newline;

    /* Test 3 */
    print_str("Test 3: Join with already joined thread (this may also give "
              "undefined results)\n");
    /* Create the thread */
    debug_str("thread_main() created thread1() as one one thread\n");
    thread_create(&td1, thread1, NULL, THREAD_TYPE_ONE_ONE);
    /* Join with the thread */
    debug_str("thread_main() called join on thread1() first time\n");
    thread_join(td1, NULL);
    /* Join again with the thread */
    debug_str("thread_main() called join on thread1() second time\n");
    if (thread_join(td1, NULL) == THREAD_SUCCESS) {

        /* Print the information */
        print_fail(3);
    } else {

        /* Check the errno */
        if (thread_errno == EINVAL) {

            /* Print the information */
            debug_str("thread_main() failed to join with thread1() second "
                      "time with error number EINVAL\n");
            print_succ(3);
        } else {

            /* Print the information */
            print_fail(3);
        }
    }

    newline;

    /* Test 4 */
    print_str("Test 4: Join with a thread for which another thread is "
              "already waiting\n");
    /* Create a target thread */
    debug_str("thread_main() created thread1() as many many thread\n");
    thread_create(&td1, thread1, NULL, THREAD_TYPE_MANY_MANY);
    /* Create another thread which will wait on the target thread */
    debug_str("thread_main() created thread2() as many many thread\n");
    thread_create(&td2, thread2, &td1, THREAD_TYPE_MANY_MANY);
    /* Join with the target thread */
    debug_str("thread_main() called join on thread1()\n");
    thread_join(td1, NULL);
    thread_join(td2, NULL);

    newline;

    /* Test 5 */
    print_str("Test 5: Join with itself\n");
    /* Join with itself */
    debug_str("thread_main() called join on thread_main()\n");
    if (thread_join(td_me, NULL) == THREAD_SUCCESS) {

        /* Print the information */
        print_fail(5);
    } else {

        /* Check the errno */
        if (thread_errno == EDEADLK) {

            /* Print the information */
            debug_str("thread_main() failed to join with itself with "
                      "error number EDEADLK\n");
            print_succ(5);
        } else {

            /* Print the information */
            print_fail(5);
        }
    }

    newline;

    /* Test 6 */
    print_str("Test 6: Mutual join between the caller and target thread\n");
    /* Create the thread */
    debug_str("thread_main() created thread3() as one one thread\n");
    thread_create(&td3, thread3, &td_me, THREAD_TYPE_ONE_ONE);
    /* Call join */
    debug_str("thread_main() called join on thread3()\n");
    thread_join(td3, NULL);

    newline;

    return NULL;
}
