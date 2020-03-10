#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "./thread.h"

/* Func1tion to print string */
#define print(str) write(1, str, strlen(str))

int func1(void *arg) {

    print("In thread1\n");

    thread_exit(-1);
}

int func2(void *arg) {

    print("In thread2\n");

    thread_exit(456);
}

void main() {

    Thread td1, td2;
    int ret1, ret2;

    thread_create(&td1, func1, NULL);
    thread_create(&td2, func2, NULL);

    thread_join(td1, &ret1);
    thread_join(td2, &ret2);

    print("In main\n");

    printf("%d\n", ret1);
    printf("%d\n", ret2);
}
