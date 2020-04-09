#ifndef _LIST_H_
#define _LIST_H_

#include <stddef.h>
#include <assert.h>

/**
 * Doubly linked list member to chain different structures
 *
 * @note Insert this as a structure member which needs to be chained as a list
 */
typedef struct ListMember {

    /* Forward pointer */
    struct ListMember *next;

    /* Backward pointer */
    struct ListMember *prev;

} ListMember;

/**
 * Doubly linked list structure
 */
typedef struct List {

    /* Pointer to the first list member */
    ListMember *head;

    /* Pointer to the last list member */
    ListMember *tail;

} List;

/**
 * Functions used internally
 *
 * @note One who feels himself/herself to be worthy shall be the one
 *       to use these functions directly else use the macros defined below
 */
void do_list_enqueue(List *list, ListMember *new);

ListMember *do_list_dequeue(List *list);

/**
 * @brief Enqueue a new node to the list
 *
 * @param[in] list Pointer to the list instance
 * @param[in] new Pointer to the any structure to be added
 * @param[in] mem Name of the ListMember member in the structure type of #new
 */
#define list_enqueue(list, new, mem)                \
    {                                               \
        assert((list));                             \
        assert((new));                              \
                                                    \
        do_list_enqueue((list), &(new)->mem);       \
    }                                               \

/**
 * @brief Dequeue a node from the list
 *
 * @param[in] list Pointer to the list instance
 * @param[in] type Type of the structure to be returned
 * @param[in] mem Name of the ListMember member in the structure of given type
 * @return Pointer to the structure containing the head ListMember
 */
#define list_dequeue(list, type, mem)                           \
    ({                                                          \
        assert((list));                                         \
        assert((list)->head);                                   \
                                                                \
        int _offset = offsetof(type, mem);                      \
                                                                \
        (type *)((void *)do_list_dequeue((list)) - _offset);    \
    })

/* List initializer */
#define LIST_INITIALIZER (List){NULL, NULL}

/**
 * @brief Initializes the linked list
 *
 * Sets the head and tail pointers of the linked list to point to nothing
 *
 * @param[out] list Pointer to the list instance
 */
static inline void list_init(List *list) {

    /* Check for errors */
    assert(list);

    /* Set the links to null */
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
static inline int list_is_empty(List *list) {

    /* Check for errors */
    assert(list);

    /* Check if both the pointers are NULL */
    return (!list->head && !list->tail);
}

#endif
