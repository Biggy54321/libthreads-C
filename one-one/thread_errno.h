#ifndef _THREAD_ERRNO_H_
#define _THREAD_ERRNO_H_

#include <errno.h>

#include "./thread_cntl.h"

/* Return values of most of the thread library functions */
#define THREAD_SUCCESS  ((int)0)
#define THREAD_FAIL     ((int)-1)

/* Define the thread specific errno macro */
#define th_errno (thread_self()->error)

/* Macro to set the errno and return failure */
#define THREAD_RET_FAIL(err)                    \
    {                                           \
        /* Set the argument error number */     \
        th_errno = (err);                       \
                                                \
        /* Return fail */                       \
        return THREAD_FAIL;                     \
    }

#endif
