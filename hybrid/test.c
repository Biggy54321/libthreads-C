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

    Thread t2;

    print("Hello, world1\n");

    thread_create(&t2, thread2, NULL, THREAD_TYPE_ONE_ONE);

    thread_join(t2, NULL);

    thread_exit(NULL);
}

void handler(int sig) {

    print("Inside the handler\n");

    thread_exit(NULL);
}

void main() {

    Thread t1;

    signal(SIGUSR1, handler);

    thread_init(1);

    thread_create(&t1, thread1, NULL, THREAD_TYPE_ONE_ONE);

    thread_kill(t1, SIGUSR1);

    thread_join(t1, NULL);

    thread_deinit();
}
