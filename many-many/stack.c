#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "./stack.h"

/**
 * Page size in bytes
 */
#define PAGE_SIZE (getpagesize())

/**
 * @brief Returns the stack limit of a thread in bytes
 * @return Size in bytes
 */
static long _stack_get_limit(void) {

    struct rlimit stack_limit;

    /* Get the stack resource limit */
    getrlimit(RLIMIT_STACK, &stack_limit);

    /* Return the current limit */
    return stack_limit.rlim_cur;
}

/**
 * @brief Allocates the stack
 * @param[out] stack Pointer to the stack instance to be initialized
 */
void stack_alloc(stack_t *stack) {

    /* Get the stack limit */
    stack->ss_size = _stack_get_limit();

    /* Memory map the stack */
    stack->ss_sp = mmap(NULL,
                        stack->ss_size + PAGE_SIZE,
                        PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK,
                        -1, 0);

    /* Set the stack guard */
    mprotect(stack->ss_sp, PAGE_SIZE, PROT_NONE);

    /* Update the new base of the stack */
    stack->ss_sp += PAGE_SIZE;

    /* Set no flags */
    stack->ss_flags = 0;
}

/**
 * @brief Dellocates the stack
 * @param[out] stack Pointer to the stack instance to be deinitialized
 */
void stack_free(stack_t *stack) {

    munmap(stack->ss_sp - PAGE_SIZE, stack->ss_size + PAGE_SIZE);
}

