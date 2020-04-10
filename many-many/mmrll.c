#include "./mods/list.h"
#include "./mods/lock.h"
#include "./thread_descr.h"
#include "./mmrll.h"

/* Many-many ready threads linked list */
static List mmrll;
/* Many-many ready threads linked list lock */
static Lock mmrll_lk;

/**
 * @brief Initialize the many-many ready list
 */
void mmrll_init(void) {

    /* Initialize the list */
    list_init(&mmrll);

    /* Initialize the lock */
    lock_init(&mmrll_lk);
}

/**
 * @brief Dequeue a thread descriptor from the many-many ready list
 * @return Thread handle
 */
Thread mmrll_dequeue(void) {

    /* Dequeue the first thread descriptor from the list */
    return list_dequeue(&mmrll, struct Thread, mmrll_mem);
}

/**
 * @brief Enqueue a thread descriptor to the many-many ready list
 * @param[in] thread Thread handle
 */
void mmrll_enqueue(Thread thread) {

    /* Add a thread descriptor to the list */
    list_enqueue(&mmrll, thread, mmrll_mem);
}

/**
 * @brief Is the many-many ready list empty
 * @return 0 if list is not empty
 * @return 1 if list is empty
 */
int mmrll_is_empty(void) {

    /* Check if list is empty */
    return list_is_empty(&mmrll);
}

/**
 * @brief Acquire the lock for the many-many ready list
 */
void mmrll_lock(void) {

    /* Acquire the lock */
    lock_acquire(&mmrll_lk);
}

/**
 * @brief Release the lock for the many-many ready list
 */
void mmrll_unlock(void) {

    /* Release the lock */
    lock_release(&mmrll_lk);
}
