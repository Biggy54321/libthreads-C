#include <stddef.h>
#include "./print.h"
#include "./print_ext.h"
#include <thread.h>

/**
 * One one yser thread
 */
void *thread(void *arg) {

    /* Print information */
    debug_str("In thread(), calling yield()\n");

    /* Call yield */
    thread_yield();

    /* Print information */
    debug_str("In thread() after yield()\n");

    return NULL;
}

/**
 * Main thread
 */
void *thread_main(void *arg) {

    Thread td;

    /* Print information */
    print_str("Thread yield testing\n\n");

    /* Test 1 */
    print_str("Test: One one thread and many many thread calling yield\n");

    /* Create a user thread */
    debug_str("thread_main() created thread() of type one one\n");
    thread_create(&td, thread, NULL, THREAD_TYPE_ONE_ONE);

    /* Join with the created thread */
    debug_str("thread_main() called join on thread()\n");
    thread_join(td, NULL);

    /* Create a user thread */
    debug_str("thread_main() created thread() of type many many\n");
    thread_create(&td, thread, NULL, THREAD_TYPE_ONE_ONE);

    /* Join with the created thread */
    debug_str("thread_main() called join on thread()\n");
    thread_join(td, NULL);

    /* Print information */
    print_str("Yield test succeeded\n");

    return NULL;
}
