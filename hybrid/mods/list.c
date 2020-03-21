#include "./list.h"

/**
 * @brief Add node to tail
 *
 * Adds a new node to the tail of the linked list
 *
 * @param[in/out] list Pointer to the list instance
 * @param[in/out] new Pointer to the list member structure
 */
void do_list_enqueue(List *list, ListMember *new) {

    /* Set the next of the new node to NULL */
    new->next = NULL;

    /* If the tail points to no one */
    if (!list->tail) {

        /* Initialize the head */
        list->head = new;
    } else {

        /* Update the tail */
        list->tail->next = new;
    }

    /* Update the prev of the new node */
    new->prev = list->tail;

    /* Update the tail */
    list->tail = new;
}

/**
 * @brief Delete the head
 *
 * Deletes and returns the current head of the linked list
 *
 * @param[in/out] list Pointer to the list instance
 * @return Pointer to the head list member
 */
ListMember *do_list_dequeue(List *list) {

    ListMember *head;

    /* Get the head member */
    head = list->head;

    /* Update the head */
    list->head = list->head->next;

    /* If the new head is NULL */
    if (!list->head) {

        /* Update the tail */
        list->tail = NULL;
    } else {

        /* Update the prev of new head */
        list->head->prev = NULL;
    }

    /* Set the next of old head to NULL */
    head->next = NULL;

    return head;
}

/**
 * @brief Initialize the list
 *
 * Initializes the list to hold zero elements
 *
 * @param[out] list Pointer to the list instance
 */
void list_init(List *list) {

    /* Set the head and tail to null */
    list->head = list->tail = NULL;
}

/**
 * @brief Is list empty
 *
 * Checks if the list is empty by conforming if the head and tail pointers are
 * pointing to anything or not
 *
 * @param[in] list Pointer to the list instance
 * @return 0 if not empty
 * @return 1 if empty
 */
int list_is_empty(List *list) {

    /* Check for errors */
    assert(list);

    /* Check if both the pointers are NULL */
    return (!list->head && !list->tail);
}
