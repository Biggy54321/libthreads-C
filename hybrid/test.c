#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

#include "./hthread.h"

#define print(str) write(1, str, strlen(str))

void handler1(int sig) {

    print("Inside the handler1\n");

    while (1);
}

void handler2(int sig) {

    print("Inside the handler2\n");

    hthread_exit((void *)1024);
}


void *func1(void *arg) {

    print("Inside func1\n");

    while (1);

    hthread_exit((void *)128);
}

void *func2(void *arg) {

    print("Inside func2\n");

    hthread_exit((void *)256);
}

void *func3(void *arg) {

    print("Inside func3\n");

    hthread_exit((void *)512);
}

void main() {

    HThread t1, t2, t3;
    void *r1, *r2, *r3;

    hthread_init(3);

    signal(SIGUSR1, handler1);
    signal(SIGUSR2, handler2);

    t1 = hthread_create(func1, NULL, HTHREAD_TYPE_ONE_ONE);
    t2 = hthread_create(func2, NULL, HTHREAD_TYPE_MANY_MANY);
    t3 = hthread_create(func3, NULL, HTHREAD_TYPE_MANY_MANY);

    hthread_kill(t1, SIGUSR1);
    hthread_kill(t1, SIGUSR2);

    hthread_join(t1, &r1);
    hthread_join(t2, &r2);
    hthread_join(t3, &r3);

    printf("%d\n", r1);
    printf("%d\n", r2);
    printf("%d\n", r3);

    hthread_deinit();
}
