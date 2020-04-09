#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>

#include "./thread.h"

#include <sys/types.h>
#include <signal.h>
#include "./mods/utils.h"

ThreadMutex mut = THREAD_MUTEX_INITIALIZER;
ThreadCond cond = THREAD_CONDITION_INITIALIZER;

#define print(str) write(STDOUT_FILENO, str, strlen(str))

void *func2(void *arg);

void *func1(void *arg) {

    Thread td2;

    thread_create(&td2, func2, NULL);

    thread_join(td2, NULL);

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

    thread_create(&td1, func1, NULL);

    thread_join(td1, NULL);

    thread_exit((void *)12);
}
