#include <stddef.h>

#include "./lock.h"
#include "./list.h"

/* List head pointer */
static Thread _head = NULL;
/* List tail pointer */
static Thread _tail = NULL;
/* List lock */
static Lock _lock = LOCK_NOT_ACQUIRED;

/**
 * @brief Adds a new  thread to the list
 * @param[in] thread New thread to be added
 * @note Assumes that the thread is already allocated
 */
void list_enqueue(Thread thread) {

    /* Check for errors */
    assert(thread);

    /* Set the next pointer of the new thread to NULL */
    thread->next = NULL;

    /* If the list is empty */
    if (!_tail) {

        /* Set the head pointer to the current thread */
        _head = thread;
    } else {

        /* Update the current tail's next */
        _tail->next = thread;
    }

    /* Update the new thread's prev */
    thread->prev = _tail;

    /* Update the tail pointer */
    _tail = thread;
}

/**
 * @brief Removes the thread from the list
 * @return Thread handle
 */
Thread list_dequeue(void) {

    Thread thread;

    /* Check for errors */
    assert(_head);

    /* Get the thread pointed by head */
    thread = _head;

    /* Update the head */
    _head = _head->next;

    /* If the new head is NULL */
    if (!_head) {

        /* Update the tail */
        _tail = NULL;
    } else {

        /* Upate the new heads prev */
        _head->prev = NULL;
    }

    /* Initialize the return threads pointers to NULL*/
    thread->next = NULL;

    return thread;
}

/**
 * @brief Checks if the thread list empty
 * @return 0 if not empty
 * @return 1 if empty
 */
int list_is_empty(void) {

    /* Return true if both head and tail are null */
    return (!_head && !_tail);
}

/**
 * @brief Acquires the lock for the threads linked list
 */
void list_lock(void) {

    /* Acquire the spinlock */
    lock_acquire(&_lock);
}

/**
 * @brief Releases the lock for the threads linked list
 */
void list_unlock(void) {

    /* Release the spinlock */
    lock_release(&_lock);
}
