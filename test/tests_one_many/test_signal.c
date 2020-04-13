#include <stddef.h>
#include <signal.h>
#include "./print.h"
#include "./print_ext.h"
#include <thread.h>

/**
 * Macro to introduce some dummy work
 */
#define DELAY()                                 \
    {                                           \
        for (int i = 0; i < 10000000; i++) {    \
                                                \
            /* Do some work */                  \
            i++;                                \
            i++;                                \
            i--;                                \
            i--;                                \
        }                                       \
    }

/**
 * SIGUSR1 signal handler
 */
void sigusr1_handler(int sig) {

    /* Print information */
    debug_str("In SIGUSR1 handler, exiting from the thread\n");

    /* Exit from the thread */
    thread_exit(NULL);
}

/**
 * SIGUSR2 signal handler
 */
void sigusr2_handler(int sig) {

    /* Print information */
    debug_str("In SIGUSR2 handler, waiting infinitely\n");

    /* Wait infinitely */
    while (1);
}

/**
 * User thread 1
 */
void *thread1(void *arg) {

    /* Print information */
    debug_str("In thread1(), waiting for signal\n");

    /* Wait */
    while (1);

    return NULL;
}

/**
 * User thread 2
 */
void *thread2(void *arg) {

    Thread *td;

    /* Get the thread descriptor argument */
    td = arg;

    /* Print information */
    debug_str("In thread2(), sending SIGUSR2 to thread_main()\n");

    /* Send the signal */
    thread_kill(*td, SIGUSR2);

    return NULL;
}

/**
 * Main thread
 */
void *thread_main(void *arg) {

    Thread td1, td2, td_me;
    sigset_t block_set;

    /* Get the thread handle */
    td_me = thread_self();

    /* Update the signal disposition for SIGUSR1 */
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGUSR2, sigusr2_handler);

    /* Print information */
    print_str("Thread signal handling testing\n\n");

    /* Test 1 */
    print_str("Test 1: Sending a signal to an infinitely sleeping thread\n");
    /* Create thread */
    debug_str("thread_main() created thread1()\n");
    thread_create(&td1, thread1, NULL);
    /* Add delay */
    DELAY();
    /* Send signal to the thread */
    debug_str("thread_main() sent SIGUSR1 to thread1()\n");
    if (thread_kill(td1, SIGUSR1) == THREAD_FAIL) {

        /* Print information */
        print_fail(1);
        thread_exit(NULL);
    } else {
        /* Join with thread */
        debug_str("thread_main() called join on thread1()\n");
        thread_join(td1, NULL);
        /* Print information */
        print_succ(1);
    }

    newline;

    /* Test 2 */
    print_str("Test 2: Blocking a signal in a thread whose handler may cause "
              "infinite wait of the target thread\n");
    /* Block SIGUSR2 before creating the thread */
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGUSR2);
    thread_sigmask(SIG_BLOCK, &block_set, NULL);
    /* Create a thread */
    debug_str("thread_main() created thread2()\n");
    thread_create(&td2, thread2, &td_me);
    /* Join with the thread */
    thread_join(td2, NULL);
    /* Print information */
    debug_str("thread_main() did not receive SIGUSR2 sent by thread2()\n");
    print_succ(2);

    return NULL;
}
