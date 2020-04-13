#include <stddef.h>
#include "./print.h"
#include "./print_ext.h"
#include <thread.h>

/**
 * User thread
 */
void *thread(void *arg) {

    /* Print information */
    debug_str("In thread() before yield()\n");

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

    /* Create a user thread */
    debug_str("thread_main() created thread()\n");
    thread_create(&td, thread, NULL);

    /* Join with the created thread */
    debug_str("thread_main() called join on thread()\n");
    thread_join(td, NULL);

    /* Print information */
    debug_str("In thread_main() before yield()\n");

    /* Call yield */
    thread_yield();

    /* Print information */
    debug_str("In thread_main() after yield()\n");

    /* Print information */
    print_str("Yield test succeeded\n");

    return NULL;
}
