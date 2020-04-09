#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "./stack.h"

/* Page size */
#define _PAGE_SIZE              (getpagesize())
/* Stack protection flags */
#define _STACK_PROT_FLAGS       (PROT_READ | PROT_WRITE)
/* Stack map flags */
#define _STACK_MAP_FLAGS        (MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK)
/* Stack guard flags */
#define _STACK_GUARD_PROT_FLAGS (PROT_NONE)

/**
 * @brief Get stack limit
 *
 * Returns the maximum stack size that can be allocated for a thread in bytes
 *
 * @return Size in bytes
 */
static long _stack_limit(void) {

    struct rlimit stack_limit;

    /* Get the stack resource limit */
    getrlimit(RLIMIT_STACK, &stack_limit);

    /* Return the stack limit */
    return stack_limit.rlim_cur;
}

/**
 * @brief Allocates stack
 *
 * Memory maps a region in virtual address space to be used a stack. Prevents
 * uncontrolled growth of the stack by allocating stack guard equal to page size
 * at the end of the stack
 *
 * @param[out] stack Pointer to the stack instance to be initialized
 */
void stack_alloc(stack_t *stack) {

    /* Check for errors */
    assert(stack);

    /* Get the stack limit */
    stack->ss_size = _stack_limit();

    /* Memory map the stack */
    stack->ss_sp = mmap(NULL,
                        stack->ss_size + _PAGE_SIZE,
                        _STACK_PROT_FLAGS,
                        _STACK_MAP_FLAGS,
                        -1, 0);

    /* Check for errors */
    assert(stack->ss_sp != MAP_FAILED);

    /* Set the stack guard */
    mprotect(stack->ss_sp, _PAGE_SIZE, _STACK_GUARD_PROT_FLAGS);

    /* Update the new base of the stack */
    stack->ss_sp += _PAGE_SIZE;

    /* Set no flags */
    stack->ss_flags = 0;
}

/**
 * @brief Dellocates the stack
 *
 * Deallocates the stack previously allocated
 *
 * @param[out] stack Pointer to the stack instance to be deinitialized
 */
void stack_free(stack_t *stack) {

    /* Check for errors */
    assert(stack);

    /* Unmap the previously mapped stack region */
    munmap(stack->ss_sp - _PAGE_SIZE, stack->ss_size + _PAGE_SIZE);
}
