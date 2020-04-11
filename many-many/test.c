#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "./thread.h"

#define print(str) (write(1, str, strlen(str)))

int c = 0, c1 = 0, c2 = 0;
ThreadSpinLock spin;
ThreadMutex mutex;

void *thread2(void *arg) {

    int i = 0;

    print("Inside thread2\n");

    while (i < 1000000) {

        /* thread_spin_lock(&spin); */
        thread_mutex_lock(&mutex);

        c++;
        c2++;

        /* thread_spin_unlock(&spin); */
        thread_mutex_unlock(&mutex);

        i++;
    }

    thread_exit(NULL);
}

void *thread1(void *arg) {

    /* int i = 0; */

    print("Inside thread\n");
    /* while (i < 1000000) { */

    /*     thread_mutex_lock(&mutex); */
    /*     /\* thread_spin_lock(&spin); *\/ */

    /*     c++; */
    /*     c1++; */

    /*     thread_mutex_unlock(&mutex); */
    /*     /\* thread_spin_unlock(&spin); *\/ */
    /*     i++; */
    /* } */

    thread_exit(NULL);
}

void handler(int sig) {

    print("Inside the handler\n");

    thread_exit(NULL);
}

void *thread_main(void *arg) {

    Thread t[500];

    print("Inside main\n");

    for (int i = 0; i < 500; i++) {

        thread_create(&t[i], thread1, NULL);
    }

    for (int i = 0; i < 500; i++) {

        thread_join(t[i], NULL);
    }

    /* thread_spin_init(&spin); */
    /* thread_mutex_init(&mutex); */

    /* print("Inside the main\n"); */

    /* thread_create(&t1, thread1, NULL); */
    /* thread_create(&t2, thread2, NULL); */

    /* /\* while (1); *\/ */

    /* thread_join(t1, NULL); */
    /* thread_join(t2, NULL); */

    /* thread_spin_destroy(&spin); */
    /* thread_mutex_destroy(&mutex); */
    /* printf("%d %d %d\n", c, c1 ,c2); */

    return NULL;
}
