#ifndef _THREAD_ERRNO_H_
#define _THREAD_ERRNO_H_

#include <errno.h>

int *__get_thread_errno_loc(void);

#endif
