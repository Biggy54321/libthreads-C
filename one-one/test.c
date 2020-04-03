#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "./thread.h"

#include <sys/types.h>
#include <signal.h>
#include "./mods/utils.h"

#define print(str) write(STDOUT_FILENO, str, strlen(str))

void handler(int sig) {

    print("Inside the handler\n");

    thread_exit((void *)127);
}

void *func1(void *arg) {

    sleep(5);

    print("In thread1\n");

    return (void *)128;
}

void *func2(void *arg) {

    print("In thread2\n");

    thread_exit((void *)256);
}

void main() {

    Thread td1, td2, me;
    void *ret1, *ret2;

    signal(SIGUSR1, handler);

    /* thread_kill(me, SIGUSR1); */

    /* thread_create(&td1, func1, NULL); */
    thread_create(&td2, func2, NULL);

    /* thread_kill(td1, SIGUSR1); */

    /* thread_join(td1, &ret1); */
    thread_join(td2, &ret2);

    print("In main\n");

    /* printf("%d\n", (int)ret1); */
    printf("%d\n", (int)ret2);

    thread_exit(NULL);
}
