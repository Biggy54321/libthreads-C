#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "./thread.h"

#define print(str) (write(1, str, strlen(str)))

void *thread1(void *arg);

void *thread2(void *arg) {

    Thread t2, t3, t4;

    print("Hello, world2\n");

    thread_create(&t2, thread1, NULL, THREAD_TYPE_MANY_MANY);
    thread_create(&t3, thread1, NULL, THREAD_TYPE_MANY_MANY);
    thread_create(&t4, thread1, NULL, THREAD_TYPE_MANY_MANY);

    thread_join(t2, NULL);
    print("After join\n");
    thread_join(t3, NULL);
    print("After join\n");
    thread_join(t4, NULL);
    print("After join\n");

    thread_exit(NULL);
}

void *thread1(void *arg) {

    print("Hello, world1\n");


    thread_exit(NULL);
}

void *thread_main(void *arg) {

    Thread t1, t2;

    thread_create(&t1, thread1, NULL, THREAD_TYPE_ONE_ONE);
    thread_create(&t2, thread2, NULL, THREAD_TYPE_ONE_ONE);

    thread_join(t1, NULL);
    thread_join(t2, NULL);

    return NULL;
}
