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

    /* Print information */
    print_str("Thread yield testing\n\n");

    /* Test */
    print_str("Test: Here thread_main() creates thread(). thread_main() waits "
              "for thread() initially, thread() yields during its execution "
              "and after thread() joins with thread_main(), thread_main() "
              "itself yields\n");

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
