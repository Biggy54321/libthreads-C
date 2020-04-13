#include <stddef.h>
#include "./print.h"
#include "./print_ext.h"
#include <thread.h>

/* Number of increments */
#define NB_INCRS (10000u)

/* Spinlock object */
ThreadSpinLock spin;

/* Count variables */
int cnt;                        /* Updated by all the three threads */
int cnt1;                       /* Updated by thread1 only */
int cnt2;                       /* Updated by thread2 only */
int cnt3;                       /* Updated by thread3 only */

/* Control variable */
int use_spinlock;

/**
 * User thread 1
 */
void *thread1(void *arg) {

    /* For some number of increments */
    for (int i = 0; i < NB_INCRS; i++) {

        /* If spinlock is to be used */
        if (use_spinlock) {

            /* Acquire the lock */
            thread_spin_lock(&spin);
        }

        /* Update the variables */
        cnt++;
        cnt1++;

        /* If spinlock is to be used */
        if (use_spinlock) {

            /* Release the lock */
            thread_spin_unlock(&spin);
        }
    }
}

/**
 * User thread 2
 */
void *thread2(void *arg) {

    /* For some number of increments */
    for (int i = 0; i < NB_INCRS; i++) {

        /* If spinlock is to be used */
        if (use_spinlock) {

            /* Acquire the lock */
            thread_spin_lock(&spin);
        }

        /* Update the variables */
        cnt++;
        cnt2++;

        /* If spinlock is to be used */
        if (use_spinlock) {

            /* Release the lock */
            thread_spin_unlock(&spin);
        }
    }
}

/**
 * User thread 3
 */
void *thread3(void *arg) {

    /* For some number of increments */
    for (int i = 0; i < NB_INCRS; i++) {

        /* If spinlock is to be used */
        if (use_spinlock) {

            /* Acquire the lock */
            thread_spin_lock(&spin);
        }

        /* Update the variables */
        cnt++;
        cnt3++;

        /* If spinlock is to be used */
        if (use_spinlock) {

            /* Release the lock */
            thread_spin_unlock(&spin);
        }
    }
}

/**
 * Main thread
 */
void *thread_main(void *arg) {

    Thread td1, td2, td3;

    /* Print information */
    print_str("Thread spinlock testing\n\n");

    /* Test 1 */
    print_str("Test 1: Create three threads, where each thread runs for some "
              "number of iterations. Each thread updates a variable local to "
              "itself and each of the thread also updates a variable which is "
              "shared amongst all. However the race on the shared variable is "
              "not handled in this case\n");
    /* Initialize the counts to all zeroes */
    debug_str("Set cnt, cnt1, cnt2 and cnt3 all to zero\n");
    cnt = cnt1 = cnt2 = cnt3 = 0;
    /* Set the locks usage to zero */
    use_spinlock = 0;
    /* Create threads */
    debug_str("thread_main() created three threads\n");
    thread_create(&td1, thread1, NULL);
    thread_create(&td2, thread2, NULL);
    thread_create(&td3, thread3, NULL);
    /* Wait for the threads */
    debug_str("thread_main() called join on the threads\n");
    thread_join(td1, NULL);
    thread_join(td2, NULL);
    thread_join(td3, NULL);
    /* Check for the consistency of the globals */
    if (cnt != (cnt1 + cnt2 + cnt3)) {

        /* Print information */
        debug_str("cnt = "); debug_int(cnt); debug_newline;
        debug_str("cnt1 = "); debug_int(cnt1); debug_newline;
        debug_str("cnt2 = "); debug_int(cnt2); debug_newline;
        debug_str("cnt3 = "); debug_int(cnt3); debug_newline;
        debug_str("The globals are not consistent\n");
        print_succ(1);
    } else {

        /* Print information */
        debug_str("Luckily the globals were found to be consistent\n");
        print_fail(1);
    }

    newline;

    /* Test 2 */
    print_str("Test 2: Create three threads, where each thread runs for some "
              "number of iterations. Each thread updates a variable local to "
              "itself and each of the thread also updates a variable which is "
              "shared amongst all. The race on the shared variable is "
              "handled in this case using spinlocks\n");
    /* Initialize the counts to all zeroes */
    debug_str("Set cnt, cnt1, cnt2 and cnt3 all to zero\n");
    cnt = cnt1 = cnt2 = cnt3 = 0;
    /* Set the locks usage to zero */
    use_spinlock = 1;
    /* Initialize the spinlock */
    debug_str("thread_main() initialized spinlock\n");
    thread_spin_init(&spin);
    /* Create threads */
    debug_str("thread_main() created three threads\n");
    thread_create(&td1, thread1, NULL);
    thread_create(&td2, thread2, NULL);
    thread_create(&td3, thread3, NULL);
    /* Wait for the threads */
    debug_str("thread_main() called join on the threads\n");
    thread_join(td1, NULL);
    thread_join(td2, NULL);
    thread_join(td3, NULL);
    /* Destroy the spinlock */
    debug_str("thread_main() deinitialized spinlock\n");
    thread_spin_destroy(&spin);
    /* Check for the consistency of the globals */
    if (cnt != (cnt1 + cnt2 + cnt3)) {

        /* Print information */
        print_fail(2);
    } else {

        /* Print information */
        debug_str("cnt = "); debug_int(cnt); debug_newline;
        debug_str("cnt1 = "); debug_int(cnt1); debug_newline;
        debug_str("cnt2 = "); debug_int(cnt2); debug_newline;
        debug_str("cnt3 = "); debug_int(cnt3); debug_newline;
        debug_str("The globals are consistent\n");
        print_succ(2);
    }

    return NULL;
}
