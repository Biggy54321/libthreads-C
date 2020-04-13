#ifndef _PRINT_EXT_H_
#define _PRINT_EXT_H_

/**
 * Debug string print macros
 */
#ifdef BLOCK_DEBUG_PRINTS
#define debug_str(str) {;}
#else
#define debug_str(str)                          \
    {                                           \
        /* Print debug header */                \
        print_str("Debug: ");                   \
                                                \
        /* Print the string */                  \
        print_str(str);                         \
    }
#endif

/**
 * Debug int print macros
 */
#ifdef BLOCK_DEBUG_PRINTS
#define debug_int(val) {;}
#else
#define debug_int(val)                          \
    {                                           \
        /* Print the integer */                 \
        __print_int(val);                       \
    }
#endif

/**
 * Debug int print macros
 */
#ifdef BLOCK_DEBUG_PRINTS
#define debug_newline {;}
#else
#define debug_newline newline
#endif

/**
 * Success print
 */
#define print_succ(test_num)                            \
    {                                                   \
        /* Print the string prior to test number */     \
        print_str("Test ");                             \
                                                        \
        /* Print the test number */                     \
        __print_int(test_num);                          \
                                                        \
        /* Print the string after the test number */    \
        print_str(": Succeeded\n");                     \
    }

/**
 * Failure print
 */
#define print_fail(test_num)                            \
    {                                                   \
        /* Print the string prior to test number */     \
        print_str("Test ");                             \
                                                        \
        /* Print the test number */                     \
        __print_int(test_num);                          \
                                                        \
        /* Print the string after the test number */    \
        print_str(": Failed\n");                        \
    }

#endif
