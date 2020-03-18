#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "./list.h"
#include "./init.h"
#include "./sched.h"

#define print(str) (write(1, str, strlen(str)))

void *func1(void *arg) {

    print("Inside func1\n");

    while (1);

    return NULL;
}

void *func2(void *arg) {

    print("Inside func2\n");

    return NULL;
}

void *func3(void *arg) {

    print("Inside func3\n");

    return NULL;
}

int main() {

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

    return 0;
}
