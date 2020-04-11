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
int c = 0, c1 = 0, c2 = 0;

#define print(str) write(STDOUT_FILENO, str, strlen(str))

void *func1(void *arg) {

    int i = 0;

    while (i < 1000000) {

        thread_mutex_lock(&mut);

        c++;
        c1++;

        thread_mutex_unlock(&mut);

        i++;
    }

    return NULL;
}

void *func2(void *arg) {

    int i = 0;

    while (i < 1000000) {

        thread_mutex_lock(&mut);

        c++;
        c2++;

        thread_mutex_unlock(&mut);

        i++;
    }


    return NULL;
}

void main() {

    Thread td1, td2, me;
    void *ret1, *ret2;

    thread_create(&td1, func1, NULL);
    thread_create(&td2, func2, NULL);

    thread_join(td1, NULL);
    thread_join(td2, NULL);

    thread_exit((void *)12);
}
