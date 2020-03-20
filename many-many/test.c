#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "./thread.h"

#define print(str) (write(1, str, strlen(str)))

void *func1(void *arg) {

    print("Inside func1\n");

    thread_exit((void *)128);

    return (void *)128;
}

void *func2(void *arg) {

    print("Inside func2\n");

    return (void *)256;
}

void *func3(void *arg) {

    print("Inside func3\n");

    return (void *)512;
}

int main() {

    Thread t1, t2, t3;
    void *ret1, *ret2, *ret3;

    thread_lib_init();

    /* Create the threads */
    thread_create(&t1, func1, NULL);
    thread_create(&t2, func2, NULL);
    thread_create(&t3, func3, NULL);

    /* Wait for the threads execution */
    thread_join(t1, &ret1);
    thread_join(t2, &ret2);
    thread_join(t3, &ret3);

    printf("%d\n", (int)ret1);
    printf("%d\n", (int)ret2);
    printf("%d\n", (int)ret3);

    thread_lib_deinit();

    return 0;
}
