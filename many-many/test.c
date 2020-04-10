#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "./thread.h"

#define print(str) (write(1, str, strlen(str)))

int c = 0, c1 = 0, c2 = 0;
ThreadSpinLock spin;

void *thread2(void *arg) {

    int i = 0;

    while (i < 1000000) {

        thread_spin_lock(&spin);

        c++;
        c2++;

        thread_spin_unlock(&spin);
        i++;
    }

    thread_exit(NULL);
}

void *thread1(void *arg) {

    int i = 0;

    while (i < 1000000) {

        thread_spin_lock(&spin);

        c++;
        c1++;

        thread_spin_unlock(&spin);
        i++;
    }

    thread_exit(NULL);
}

void handler(int sig) {

    print("Inside the handler\n");

    thread_exit(NULL);
}

void *thread_main(void *arg) {

    Thread t1, t2;

    thread_spin_init(&spin);

    thread_create(&t1, thread1, NULL);
    thread_create(&t2, thread2, NULL);

    thread_join(t1, NULL);
    thread_join(t2, NULL);

    thread_spin_destroy(&spin);

    /* printf("%d %d %d\n", c, c1 ,c2); */

    return NULL;
}
/* Creating many-many threads in one-one thread fails badly */
/* Initialize all the fields in the event part of the timer */
