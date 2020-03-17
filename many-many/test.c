#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "./list.h"
#include "./init.h"
#include "./sched.h"

#define print(str) (write(1, str, strlen(str)))

void *func1(void *arg) {

    while (1);

    print("Inside func1\n");
}

void *func2(void *arg) {

    print("Inside func2\n");
}

void *func3(void *arg) {

    print("Inside func3\n");
}

void main() {

    Thread t1, t2, t3;

    /* Create threads */
    init_thread(&t1, func1, NULL);
    init_thread(&t2, func2, NULL);
    init_thread(&t3, func3, NULL);

    /* Add the threads to the list */
    list_lock();
    list_enqueue(t1);
    list_enqueue(t2);
    list_enqueue(t3);
    list_unlock();

    /* Start the kernel threads */
    sched_init();

    /* Wait */
    while (1);

    /* Stop the kernel threads */
    sched_deinit();
}
