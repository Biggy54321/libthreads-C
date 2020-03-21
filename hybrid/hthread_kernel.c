#define _GNU_SOURCE
#include <sched.h>
#include <stdlib.h>
#include <assert.h>

#include "./lib/utils.h"
#include "./lib/list.h"
#include "./lib/stack.h"
#include "./hthread_kernel.h"
#include "./hthread_sched.h"

/**
 * Kernel thread structure
 */
typedef struct _KThread {

    /* Kernel thread id */
    int id;

    /* Wait word */
    int wait;

    /* Thread stack */
    stack_t stack;

    /* List member */
    ListMember list_mem;
} KThread;

/* Global list of kernel thread structures */
static List _kthread_list;

/**
 * @brief Create a kernel thread
 * @return Returns the pointer to the kernel thread structure
 */
static KThread *_kernel_thread_create(void) {

    KThread *kthread;

    /* Allocate the kernel thread structure */
    kthread = (KThread *)malloc(sizeof(KThread));
    /* Check for errors */
    assert(kthread);

    /* Allocate a stack */
    stack_alloc(&kthread->stack);

    /* Create a kernel thread */
    kthread->id = clone(hthread_sched_dispatch,
                        kthread->stack.ss_sp + kthread->stack.ss_size,
                        CLONE_VM | CLONE_FS | CLONE_FILES |
                        CLONE_SIGHAND | CLONE_THREAD |
                        CLONE_SYSVSEM | CLONE_PARENT_SETTID |
                        CLONE_CHILD_CLEARTID,
                        NULL,
                        &kthread->wait,
                        NULL,
                        &kthread->wait);
    /* Check for errors */
    assert(kthread->id != -1);

    return kthread;
}

/**
 * @brief Destroy a kernel thread
 * @param[in] kthread Pointer to the kernel thread structure
 */
static void _kernel_thread_destroy(KThread *kthread) {

    /* Check for errors */
    assert(kthread);

    /* Wait for the current kernel thread to finish */
    futex(&kthread->wait, FUTEX_WAIT, kthread->id);

    /* Free the stack */
    stack_free(&kthread->stack);

    /* Free the structure */
    free(kthread);
}

/**
 * @brief Initialize the kernel threads
 *
 * Creates specified number of kernel threads. The kernel threads will be
 * responsible for scheduling the many-many type of user threads. The kernel
 * threads created will never return unless hthread_kernel_threads_deinit() is
 * called
 *
 * @param[in] nb_kthds Number of kernel threads
 * @note Should be done by the main thread
 */
void hthread_kernel_threads_init(int nb_kthds) {

    KThread *kthread;

    /* Initialize the list */
    list_init(&_kthread_list);

    /* Set the scheduling status */
    hthread_sched_start();

    /* For each kernel thread */
    for (int i = 0; i < nb_kthds; i++) {

        /* Create the kernel thread */
        kthread = _kernel_thread_create();

        /* Add the kernel thread to the list */
        list_enqueue(&_kthread_list, kthread, list_mem);
    }
}

/**
 * @brief Deinitialize the kernel threads
 *
 * Frees the resources allocated to the kernel thread. Also this function does
 * not return unless all th kernel threads have not returned
 *
 * @note Should be done by the main thread
 */
void hthread_kernel_threads_deinit(void) {

    KThread *kthread;

    /* Clear the scheduling status */
    hthread_sched_stop();

    /* While the list is not empty */
    while (!list_is_empty(&_kthread_list)) {

        /* Get the kernel thread from the list */
        kthread = list_dequeue(&_kthread_list, KThread, list_mem);

        /* Destroy the thread */
        _kernel_thread_destroy(kthread);
    }
}
