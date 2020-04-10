#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "./thread.h"

#define print(str) (write(1, str, strlen(str)))

void *thread2(void *arg) {

    print("Hello, world2\n");

    thread_exit(NULL);
}

void *thread1(void *arg) {

    Thread t2, t3, t4;

    print("Hello, world1\n");

    /* thread_create(&t2, thread2, NULL, THREAD_TYPE_MANY_MANY); */
    /* thread_create(&t3, thread2, NULL, THREAD_TYPE_MANY_MANY); */
    /* thread_create(&t4, thread2, NULL, THREAD_TYPE_MANY_MANY); */

    /* thread_join(t2, NULL); */
    /* thread_join(t3, NULL); */
    /* thread_join(t4, NULL); */

    thread_exit(NULL);
}

void handler(int sig) {

    print("Inside the handler\n");

    thread_exit(NULL);
}

void main() {

    Thread t1, t2;

    /* signal(SIGUSR1, handler); */

    thread_init(1);

    thread_create(&t1, thread1, NULL, THREAD_TYPE_ONE_ONE);
    thread_create(&t2, thread2, NULL, THREAD_TYPE_MANY_MANY);

    /* thread_kill(t1, SIGUSR1); */

    /* thread_join(t1, NULL); */
    thread_join(t2, NULL);

    thread_deinit();
}

/* Creating many-many threads in one-one thread fails badly */
/* Initialize all the fields in the event part of the timer */
