#define _GNU_SOURCE
#include <sched.h>
#include <asm/prctl.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <syscall.h>

#include "./kernel_thread.h"
#include "./scheduler.h"
#include "./stack.h"

/* Array of kernel threads */
static KernelThread _kthreads[NB_OF_KERNEL_THREADS];

/**
 * @brief Allocate the kernel thread
 * @param[out] kthread Pointer to the kernel thread instance
 */
static void _kernel_thread_create(KernelThread *kthread) {

    KernelThreadControlBlock *ktcb;
    void *stack_top;

    /* Allocate memory for the kernel thread control block */
    ktcb = (KernelThreadControlBlock *)malloc(sizeof(KernelThreadControlBlock));

    /* Set the address of the kernel thread control block */
    *kthread = (KernelThread)ktcb;

    /* Initialize the currently mapped user thread to NULL */
    ktcb->user_thread = NULL;

    /* Initialize the stack for the kernel thread */
    stack_alloc(&(ktcb->context.uc_stack));

    /* Get the stack top */
    stack_top = ktcb->context.uc_stack.ss_sp + ktcb->context.uc_stack.ss_size;

    /* Create the kernel thread */
    clone(scheduler,
          stack_top,
          CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD |
          CLONE_SYSVSEM | CLONE_PARENT_SETTID | CLONE_SETTLS,
          ktcb,
          &(ktcb->thread_id),
          ktcb,
          NULL);
}

/**
 * @brief Deallocate the kernel thread
 */
static void _kernel_thread_destroy(KernelThread *kthread) {

    ;
}

/**
 * @brief Initializes all the kernel threads
 */
void kernel_threads_init(void) {

    /* For each kernel thread */
    for (int i = 0; i < NB_OF_KERNEL_THREADS; i++) {

        _kernel_thread_create(&_kthreads[i]);
    }
}

/**
 * @brief Denitializes all the kernel threads
 */
void kernel_threads_deinit(void) {

    /* For each kernel thread */
    for (int i = 0; i < NB_OF_KERNEL_THREADS; i++) {

        _kernel_thread_destroy(&_kthreads[i]);
    }
}

/**
 * @brief Get/Set architecture specific thread state
 * @param[in] code Type of subfunction
 * @param[in/out] Address of value to be set or to be read into
 * @return 0 on success, else -1
 */
static int _arch_prctl(int code, unsigned long *addr) {

    /* Call using syscall wrapper */
    return syscall(SYS_arch_prctl, code, addr);
}

/**
 * @brief Returns the currently running kernel thread
 * @return Kernel thread instance
 */
KernelThread kernel_thread_self(void) {

    long fs;

    /* Get the contents of the fs register */
    _arch_prctl(ARCH_GET_FS, &fs);

    /* Return kernel thread handle */
    return (KernelThread)fs;

}
