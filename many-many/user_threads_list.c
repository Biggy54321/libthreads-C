#include <stddef.h>

#include "./user_threads_list.h"
#include "./spin_lock.h"

/* User linked list head pointer */
static UserThread _head = NULL;
/* User linked list tail pointer */
static UserThread _tail = NULL;
/* User linked list lock */
static int _lock = SPIN_LOCK_NOT_TAKEN;

/**
 * @brief Adds a new user thread to the list
 * @param[in] user_thread New user thread to be added
 * @note Assumes that the user thread is already allocated
 */
void list_add_thread(UserThread user_thread) {

    /* Set the next pointer of the new thread to NULL */
    user_thread->next = NULL;

    /* If the list is empty */
    if (!_tail) {

        /* Set the head pointer to the current thread */
        _head = user_thread;
    } else {

        /* Update the current tail's next */
        _tail->next = user_thread;
    }

    /* Update the new thread's prev */
    user_thread->prev = _tail;

    /* Update the tail pointer */
    _tail = user_thread;
}

/**
 * @brief Removes the a user thread from the list
 * @return User thread
 */
UserThread list_get_thread(void) {

    UserThread return_thread;

    /* Get the user thread pointed by head */
    return_thread = _head;

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
    return_thread->next = NULL;

    return return_thread;
}

/**
 * @brief Checks if the user thread list empty
 * @return 0 if not empty
 * @return 1 if empty
 */
int list_is_empty(void) {

    /* Return true if both head and tail are null */
    return (!_head && !_tail);
}

/**
 * @brief Acquires the lock for the user threads linked list
 */
void list_lock(void) {

    /* Acquire the spinlock */
    spin_lock(&_lock);
}

/**
 * @brief Releases the lock for the user threads linked list
 */
void list_unlock(void) {

    /* Release the spinlock */
    spin_unlock(&_lock);
}
