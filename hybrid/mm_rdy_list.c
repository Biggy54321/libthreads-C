#include "./mods/list.h"
#include "./mods/lock.h"
#include "./mm_rdy_list.h"
#include "./hthread_priv.h"

/* Many-many ready thread list */
static List mm_rdy_list = LIST_INITIALIZER;
/* Many-many ready thread list lock */
static Lock mm_rdy_lock = LOCK_INITIALIZER;

/**
 * @brief Adds a thread to the thread list
 *
 * Enqueues a new hybrid thread of type many-many to the list. The thread should
 * of type many-many as the list pointers required to link the new thread are
 * needed
 *
 * @param[in] hthread Thread handle
 */
void mm_rdy_list_add(HThread hthread) {

    /* Link the list member of the structure to the global list */
    list_enqueue(&mm_rdy_list, MANY_TLS(hthread), rdy_list_mem);
}

/**
 * @brief Get a thread from the thread list
 *
 * Dequeues a hybrid thread of type many-many from the list. The thread handle
 * for the thread is returned even if it is of many-many type
 *
 * @return Thread handle
 */
HThread mm_rdy_list_get(void) {

    struct HThreadManyMany *many_many;

    /* Get the first entered thread from the list */
    many_many = list_dequeue(&mm_rdy_list, struct HThreadManyMany, rdy_list_mem);

    /* Return the thread handle */
    return BASE_TLS(many_many);
}

/**
 * @brief Checks if the list is empty
 * @return 0 if list is not empty
 * @return 1 if list is empty
 */
int mm_rdy_list_is_empty(void) {

    return list_is_empty(&mm_rdy_list);
}

/**
 * @brief Lock the global lock for list
 *
 * Acquires the lock reserved for the global thread list. Does not return
 * unless the lock is not acquired
 */
void mm_rdy_list_lock(void) {

    /* Acquire the lock */
    lock_acquire(&mm_rdy_lock);
}

/**
 * @brief Unlock the global lock for list
 *
 * Releases the lock reserved for the global thread list
 */
void mm_rdy_list_unlock(void) {

    /* Release the lock */
    lock_release(&mm_rdy_lock);
}
