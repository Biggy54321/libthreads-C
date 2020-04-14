#include <stddef.h>
#include "./print.h"
#include "./print_ext.h"
#include <thread.h>

/* Number of increments */
#define NB_INCRS (10000u)

/* Mutex object */
ThreadMutex mtx;

/* Count variables */
int cnt;                        /* Updated by all the three threads */
int cnt1;                       /* Updated by thread1 only */
int cnt2;                       /* Updated by thread2 only */
int cnt3;                       /* Updated by thread3 only */

/* Control variable */
int use_mutex;

/**
 * User thread 1
 */
void *thread1(void *arg) {

    /* For some number of increments */
    for (int i = 0; i < NB_INCRS; i++) {

        /* If mutex is to be used */
        if (use_mutex) {

            /* Acquire the lock */
            thread_mutex_lock(&mtx);
        }

        /* Update the variables */
        cnt++;
        cnt1++;

        /* If mutex is to be used */
        if (use_mutex) {

            /* Release the lock */
            thread_mutex_unlock(&mtx);
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

        /* If mutex is to be used */
        if (use_mutex) {

            /* Acquire the lock */
            thread_mutex_lock(&mtx);
        }

        /* Update the variables */
        cnt++;
        cnt2++;

        /* If mutex is to be used */
        if (use_mutex) {

            /* Release the lock */
            thread_mutex_unlock(&mtx);
        }
    }

    return NULL;
}

/**
 * User thread 3
 */
void *thread3(void *arg) {

    /* For some number of increments */
    for (int i = 0; i < NB_INCRS; i++) {

        /* If mutex is to be used */
        if (use_mutex) {

            /* Acquire the lock */
            thread_mutex_lock(&mtx);
        }

        /* Update the variables */
        cnt++;
        cnt3++;

        /* If mutex is to be used */
        if (use_mutex) {

            /* Release the lock */
            thread_mutex_unlock(&mtx);
        }
    }

    return NULL;
}

/**
 * User thread 4
 */
void *thread4(void *arg) {

    /* Print information */
    debug_str("thread4() tried to acquire the mutex owned by thread_main()\n");
    if (thread_mutex_trylock(&mtx) == THREAD_SUCCESS) {

        /* Print information */
        print_fail(5);
    } else {

        /* Check the error number */
        if (thread_errno == EBUSY) {

            /* Print information */
            debug_str("thread4() failed to try to acquire the mutex with "
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

    Thread td1, td2, td3, td4;

    /* Print information */
    print_str("Thread mutex testing\n\n");

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
    use_mutex = 0;
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
              "handled in this case using mutexes\n");
    /* Initialize the counts to all zeroes */
    debug_str("Set cnt, cnt1, cnt2 and cnt3 all to zero\n");
    cnt = cnt1 = cnt2 = cnt3 = 0;
    /* Set the locks usage to zero */
    use_mutex = 1;
    /* Initialize the mutex */
    debug_str("thread_main() initialized mutex\n");
    thread_mutex_init(&mtx);
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
    /* Destroy the mutex */
    debug_str("thread_main() deinitialized mutex\n");
    thread_mutex_destroy(&mtx);
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

    newline;

    /* Test 3 */
    print_str("Test 3: Acquire an already owned mutex\n");
    debug_str("thread_main() initialized mutex\n");
    thread_mutex_init(&mtx);
    debug_str("thread_main() acquired mutex\n");
    thread_mutex_lock(&mtx);
    debug_str("thread_main() tried to acquire the same mutex again\n");
    if (thread_mutex_lock(&mtx) == THREAD_SUCCESS) {

        /* Print information */
        debug_str("thread_main() reacquired the lock\n");
        debug_str("thread_main() released mutex\n");
        thread_mutex_unlock(&mtx);
        debug_str("thread_main() deinitialized mutex\n");
        thread_mutex_destroy(&mtx);
        print_succ(3);
    } else {

        /* Print information */
        debug_str("thread_main() failed to reacquire the lock\n");
        debug_str("thread_main() released mutex\n");
        thread_mutex_unlock(&mtx);
        debug_str("thread_main() deinitialized mutex\n");
        thread_mutex_destroy(&mtx);
        print_fail(2);
    }

    newline;

    /* Test 4 */
    print_str("Test 4: Release mutex which is not owned\n");
    debug_str("thread_main() initialized mutex\n");
    thread_mutex_init(&mtx);
    debug_str("thread_main() tried to unlock a mutex which was not locked\n");
    if (thread_mutex_unlock(&mtx) == THREAD_SUCCESS) {

        /* Print information */
        print_fail(4);
    } else {

        /* Check the error number */
        if (thread_errno == EACCES) {

            /* Print information */
            debug_str("thread_main() failed to unlock the mutex with "
                      "error number EACCES\n");
            debug_str("thread_main() deinitialized mutex\n");
            thread_mutex_destroy(&mtx);
            print_succ(4);
        } else {

            /* Print information */
            debug_str("thread_main() deinitialized mutex\n");
            thread_mutex_destroy(&mtx);
            print_fail(4);
        }
    }

    newline;

    /* Test 5 */
    print_str("Test 5: Try to acquire an already acquired mutex by other "
              "thread\n");
    debug_str("thread_main() initialized mutex\n");
    thread_mutex_init(&mtx);
    debug_str("thread_main() acquired mutex\n");
    thread_mutex_lock(&mtx);
    debug_str("thread_main() created thread4()\n");
    thread_create(&td4, thread4, NULL);
    debug_str("thread_main() called join on thread4()\n");
    thread_join(td4, NULL);
    debug_str("thread_main() released mutex\n");
    thread_mutex_unlock(&mtx);
    debug_str("thread_main() deinitialized mutex\n");
    thread_mutex_destroy(&mtx);

    return NULL;
}
