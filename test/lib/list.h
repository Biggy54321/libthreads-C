#ifndef _LIST_H_
#define _LIST_H_

#include <stddef.h>
#include <assert.h>

/**
 * Doubly linked list member of structure
 */
typedef struct _ListMember {

    /* Forward pointer */
    struct _ListMember *next;

    /* Backward pointer */
    struct _ListMember *prev;

} ListMember;

/**
 * Doubly linked list structure
 */
typedef struct _List {

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

void list_init(List *list);

int list_is_empty(List *list);

/**
 * @brief Enqueue a new node to the list
 *
 * @param[in] list Pointer to the list instance
 * @param[in] new Pointer to the any structure to be added
 * @param[in] mem Name of the ListMember member in the structure type of #new
 */
#define list_enqueue(list, new, mem)            \
    {                                           \
        assert((list));                         \
        assert((new));                          \
                                                \
        do_list_enqueue((list), &(new)->mem);   \
    }

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

#endif
