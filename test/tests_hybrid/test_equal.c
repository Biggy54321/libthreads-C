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

    Thread td_one, td_many, td_me;

    /* Print information */
    print_str("Thread equality testing\n");

    /* Test 1 */
    print_str("Test 1: Testing equality of the main thread object with itself\n");
    debug_str("thread_main() got its own thread object\n");
    td_me = thread_self();
    debug_str("thread_main() tested equality of its thread object with itself\n");
    if (thread_equal(td_me, td_me) == 1) {

        print_succ(1);
    } else {

        print_fail(1);
    }

    newline;

    /* Test 2 */
    print_str("Test 2: Testing equality of two thread objects calling the "
              "same start function but mapped in different types\n");
    debug_str("thread_main() created thread() as a one one thread\n");
    thread_create(&td_one, thread, NULL, THREAD_TYPE_ONE_ONE);
    debug_str("thread_main() created thread() as a many many thread\n");
    thread_create(&td_many, thread, NULL, THREAD_TYPE_MANY_MANY);
    debug_str("thread_main() tested equality of the two newly created thread objects\n");
    if (thread_equal(td_one, td_many) == 0) {

        print_succ(2);
    } else {

        print_fail(2);
    }

    return NULL;
}
