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

void *func1(void *arg) {

    print("In thread1\n");

    thread_mutex_lock(&mut);

    thread_cond_wait(&cond, &mut);

    thread_mutex_unlock(&mut);

    return (void *)128;
}

void *func2(void *arg) {

    print("In thread2\n");

    thread_mutex_lock(&mut);

    thread_cond_wait(&cond, &mut);

    thread_mutex_unlock(&mut);

    thread_exit((void *)256);
}

void main() {

    Thread td1, td2, me;
    void *ret1, *ret2;

    if (thread_create(&td1, func1, NULL)) {
        printf("%d\n", th_errno);
    }

    if (thread_create(&td2, func2, NULL)) {
        print("Error\n");
    }

    sleep(1);

    thread_cond_broadcast(&cond);

    thread_join(td1, &ret1);
    thread_join(td2, &ret2);

    print("In main\n");

    printf("%d\n", (int)ret1);
    printf("%d\n", (int)ret2);

    thread_exit((void *)12);
}
