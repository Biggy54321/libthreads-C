#include "./lib/list.h"
#include "./lib/lock.h"
#include "./hthread_list.h"

/* Global many many threads list  */
static List _hthread_list;
/* Lock for the global list */
static Lock _hthread_list_lock;

/**
 * @brief Initialize the thread list
 *
 * Initializes the global #hthread_list List structure. After initialization
 * the list holds zero thread information. Also initializes the lock for the list
 */
void hthread_list_init(void) {

    /* Initialize the list */
    list_init(&_hthread_list);

    /* Initialize the lock */
    lock_init(&_hthread_list_lock);
}

/**
 * @brief Adds a thread to the thread list
 *
 * Enqueues a new hybrid thread of type many-many to the list. The thread should
 * of type many-many as the list pointers required to link the new thread are
 * needed
 *
 * @param[in] hthread Thread handle
 */
void hthread_list_add(HThread hthread) {

    /* Link the list member of the structure to the global list */
    list_enqueue(&_hthread_list, MANY_MANY(hthread), list_mem);
}

/**
 * @brief Get a thread from the thread list
 *
 * Dequeues a hybrid thread of type many-many from the list. The thread handle
 * for the thread is returned even if it is of many-many type
 *
 * @return Thread handle
 */
HThread hthread_list_get(void) {

    struct _HThreadManyMany *hthread_many_many;

    /* Get the thread from the list */
    hthread_many_many = list_dequeue(&_hthread_list,
                                     struct _HThreadManyMany,
                                     list_mem);

    /* Return the thread handle */
    return BASE(hthread_many_many);
}

/**
 * @brief Checks if the list is empty
 * @return 0 if list is not empty
 * @return 1 if list is empty
 */
int hthread_list_is_empty(void) {

    return list_is_empty(&_hthread_list);
}

/**
 * @brief Lock the global lock for list
 *
 * Acquires the lock reserved for the global thread list. Does not return
 * unless the lock is not acquired
 */
void hthread_list_lock(void) {

    lock_acquire(&_hthread_list_lock);
}

/**
 * @brief Unlock the global lock for list
 *
 * Releases the lock reserved for the global thread list
 */
void hthread_list_unlock(void) {

    lock_release(&_hthread_list_lock);
}
