#include "./print.h"

/**
 * @brief Print the integer
 * @param[in] var Integer
 */
void __print_int(unsigned int var) {

    char ch;

    /* If the integer has one more digit */
    if (var / 10) {

        /* Call recursively */
        __print_int(var / 10);

    }

    /* Get the current digit */
    ch = (var % 10) + '0';

    /* Print the digit */
    syscall(SYS_write, STDOUT_FILENO, &ch, 1);
}
