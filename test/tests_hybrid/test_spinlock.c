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

    return NULL;
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

    return NULL;
}

/**
 * User thread 4
 */
void *thread3(void *arg) {

    /* Print information */
    debug_str("thread3() tried to acquire the spinlock owned by thread_main()\n");
    if (thread_spin_trylock(&spin) == THREAD_SUCCESS) {

        /* Print information */
        print_fail(5);
    } else {

        /* Check the error number */
        if (thread_errno == EBUSY) {

            /* Print information */
            debug_str("thread3() failed to try to acquire the spinlock with "
                      "error number EBUSY\n");
            print_succ(5);
        } else {

            /* Print information */
            print_fail(5);
        }
    }

    return NULL;
}

/**
 * Main thread
 */
void *thread_main(void *arg) {

    Thread td1, td2, td3;

    /* Print information */
    print_str("Thread spinlock testing\n\n");

    /* Test 1 */
    print_str("Test 1: Create two threads, where each thread runs for some "
              "number of iterations. Each thread updates a variable local to "
              "itself and each of the thread also updates a variable which is "
              "shared amongst all. However the race on the shared variable is "
              "not handled in this case\n");
    /* Initialize the counts to all zeroes */
    debug_str("Set cnt, cnt1 and cnt2 all to zero\n");
    cnt = cnt1 = cnt2 = 0;
    /* Set the locks usage to zero */
    use_spinlock = 0;
    /* Create threads */
    debug_str("thread_main() created two threads\n");
    thread_create(&td1, thread1, NULL, THREAD_TYPE_ONE_ONE);
    thread_create(&td2, thread2, NULL, THREAD_TYPE_MANY_MANY);
    /* Wait for the threads */
    debug_str("thread_main() called join on the threads\n");
    thread_join(td1, NULL);
    thread_join(td2, NULL);
    /* Check for the consistency of the globals */
    if (cnt != (cnt1 + cnt2)) {

        /* Print information */
        debug_str("cnt = "); debug_int(cnt); debug_newline;
        debug_str("cnt1 = "); debug_int(cnt1); debug_newline;
        debug_str("cnt2 = "); debug_int(cnt2); debug_newline;
        debug_str("The globals are not consistent\n");
        print_succ(1);
    } else {

        /* Print information */
        debug_str("Luckily the globals were found to be consistent\n");
        print_fail(1);
    }

    newline;

    /* Test 2 */
    print_str("Test 2: Create two threads, where each thread runs for some "
              "number of iterations. Each thread updates a variable local to "
              "itself and each of the thread also updates a variable which is "
              "shared amongst all. The race on the shared variable is "
              "handled in this case using spinlocks\n");
    /* Initialize the counts to all zeroes */
    debug_str("Set cnt, cnt1 and cnt2 all to zero\n");
    cnt = cnt1 = cnt2 = 0;
    /* Set the locks usage to zero */
    use_spinlock = 1;
    /* Initialize the spinlock */
    debug_str("thread_main() initialized spinlock\n");
    thread_spin_init(&spin);
    /* Create threads */
    debug_str("thread_main() created two threads\n");
    thread_create(&td1, thread1, NULL, THREAD_TYPE_ONE_ONE);
    thread_create(&td2, thread2, NULL, THREAD_TYPE_MANY_MANY);
    /* Wait for the threads */
    debug_str("thread_main() called join on the threads\n");
    thread_join(td1, NULL);
    thread_join(td2, NULL);
    /* Destroy the spinlock */
    debug_str("thread_main() deinitialized spinlock\n");
    thread_spin_destroy(&spin);
    /* Check for the consistency of the globals */
    if (cnt != (cnt1 + cnt2)) {

        /* Print information */
        print_fail(2);
    } else {

        /* Print information */
        debug_str("cnt = "); debug_int(cnt); debug_newline;
        debug_str("cnt1 = "); debug_int(cnt1); debug_newline;
        debug_str("cnt2 = "); debug_int(cnt2); debug_newline;
        debug_str("The globals are consistent\n");
        print_succ(2);
    }

    newline;

    /* Test 3 */
    print_str("Test 3: Acquire an already owned spinlock\n");
    debug_str("thread_main() initialized spinlock\n");
    thread_spin_init(&spin);
    debug_str("thread_main() acquired spinlock\n");
    thread_spin_lock(&spin);
    debug_str("thread_main() tried to acquire the same spinlock again\n");
    if (thread_spin_lock(&spin) == THREAD_SUCCESS) {

        /* Print information */
        debug_str("thread_main() reacquired the lock\n");
        debug_str("thread_main() released spinlock\n");
        thread_spin_unlock(&spin);
        debug_str("thread_main() deinitialized spinlock\n");
        thread_spin_destroy(&spin);
        print_succ(3);
    } else {

        /* Print information */
        debug_str("thread_main() failed to reacquire the lock\n");
        debug_str("thread_main() released spinlock\n");
        thread_spin_unlock(&spin);
        debug_str("thread_main() deinitialized spinlock\n");
        thread_spin_destroy(&spin);
        print_fail(2);
    }

    newline;

    /* Test 4 */
    print_str("Test 4: Release spinlock which is not owned\n");
    debug_str("thread_main() initialized spinlock\n");
    thread_spin_init(&spin);
    debug_str("thread_main() tried to unlock a spinlock which was not locked\n");
    if (thread_spin_unlock(&spin) == THREAD_SUCCESS) {

        /* Print information */
        print_fail(4);
    } else {

        /* Check the error number */
        if (thread_errno == EACCES) {

            /* Print information */
            debug_str("thread_main() failed to unlock the spinlock with "
                      "error number EACCES\n");
            debug_str("thread_main() deinitialized spinlock\n");
            thread_spin_destroy(&spin);
            print_succ(4);
        } else {

            /* Print information */
            debug_str("thread_main() deinitialized spinlock\n");
            thread_spin_destroy(&spin);
            print_fail(4);
        }
    }

    newline;

    /* Test 5 */
    print_str("Test 5: Try to acquire an already acquired spinlock by other "
              "thread\n");
    debug_str("thread_main() initialized spinlock\n");
    thread_spin_init(&spin);
    debug_str("thread_main() acquired spinlock\n");
    thread_spin_lock(&spin);
    debug_str("thread_main() created thread3()\n");
    thread_create(&td3, thread3, NULL, THREAD_TYPE_ONE_ONE);
    debug_str("thread_main() called join on thread3()\n");
    thread_join(td3, NULL);
    debug_str("thread_main() released spinlock\n");
    thread_spin_unlock(&spin);
    debug_str("thread_main() deinitialized spinlock\n");
    thread_spin_destroy(&spin);

    return NULL;
}
