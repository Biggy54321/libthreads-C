#include <thread.h>
#include "./print.h"
#include "./print_ext.h"

/* Create and initialize the once control object */
ThreadOnce once = THREAD_ONCE_INIT;
/* Debug variable */
int debug = 0;

/**
 * Function to be called only once
 */
void init_func(void) {

    /* Print information */
    debug_str("Increment debug variable by one\n");
    /* Increment the variable */
    debug++;
}

/**
 * User thread
 */
void *thread(void *arg) {

    /* Print information */
    debug_str("Inside thread(), invoking init_func() using a once control "
              "object\n");

    /* Call the thread once */
    thread_once(&once, init_func);

    return NULL;
}

/**
 * Main thread
 */
void *thread_main(void *arg) {

    Thread td;

    /* Print information */
    print_str("Thread once testing\n\n");

    /* Test */
    print_str("Test: Incrementing a debug variable initialized to zero inside "
              "a function bounded by a once control object, hence the variable "
              "should be incremented only once even if it is invoked multiple "
              "times in multiple threads\n");

    /* Print information */
    debug_str("Inside thread_main(), invoking init_func() using a once control "
              "object\n");
    /* Call thread once */
    thread_once(&once, init_func);

    /* Create thread */
    debug_str("thread_main() created thread()\n");
    thread_create(&td, thread, NULL, THREAD_TYPE_MANY_MANY);
    /* Join with thread */
    debug_str("thread_main() called join on thread()\n");
    thread_join(td, NULL);

    /* Check the debug variable value */
    if (debug == 1) {

        /* Print information */
        debug_str("The debug variable got incremented only once\n");
        print_str("Test succeeded\n");
    } else {

        print_str("Test failed\n");
    }

    thread_exit(NULL);
}
