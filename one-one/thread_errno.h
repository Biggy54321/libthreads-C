#ifndef _THREAD_ERRNO_H_
#define _THREAD_ERRNO_H_

#include <errno.h>

#include "./thread_cntl.h"

/* Remove the original definition of the error number */
#undef errno

/* Redefine the error number macro */
#define errno (thread_self()->errno)

/* Return values of most of the thread library functions */
#define THREAD_SUCCESS  ((int)0)
#define THREAD_FAIL     ((int)-1)

/* Macro to set the errno and return failure */
#define THREAD_RET_FAIL(err) {errno = (err), return THREAD_FAIL;}

#endif
