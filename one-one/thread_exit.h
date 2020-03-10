#ifndef _THREAD_EXIT_H_
#define _THREAD_EXIT_H_

#include "./thread_types.h"

/**
 * Thread exit definition
 */
#define thread_exit(return_value)               \
    {                                           \
        return (void *)return_value;          \
    }

#endif
