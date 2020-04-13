#ifndef _PRINT_H
#define _PRINT_H_

#include <stdio.h>
#include <syscall.h>
#include <unistd.h>
#include <string.h>

/**
 * @brief Print the string
 * @param[in] str String
 */
static inline void print_str(char *str) {

    /* Use the write system call */
    syscall(SYS_write, STDOUT_FILENO, str, strlen(str));
}

/**
 * @brief Print the integer
 * @param[in] var Unsigned integer
 */
void __print_int(unsigned int var);

/**
 * Macro to print single newline character
 */
#define newline (print_str("\n"))

/**
 * Macro to print integer
 */
#define print_int(var)                          \
    {                                           \
        /* Set the variable */                  \
        int __var = (var);                      \
                                                \
        /* If the variable is negative */       \
        if (__var < 0) {                        \
                                                \
            /* Print the minus sign */          \
            print_str("-");                     \
                                                \
            /* Make the variable positive */    \
            __var = -__var;                     \
        }                                       \
                                                \
        /* Print the variable  */               \
        __print_int(__var);                     \
                                                \
        /* Print newline */                     \
        newline;                                \
    }

#endif
