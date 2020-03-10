#include <asm/prctl.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <syscall.h>

#include "./thread_self.h"

/**
 * @brief Get/Set architecture specific thread state
 * @param[in] code Type of subfunction
 * @param[in/out] Address of value to be set or to be read into
 * @return 0 on success, else -1
 */
static int _arch_prctl(int code, unsigned long *addr) {

    /* Call using syscall wrapper */
    return syscall(SYS_arch_prctl, code, addr);
}

/**
 * @brief Returns the thread handle of the current thread
 * @return Thread handle
 * @note The thread handle will be valid only if the thread is created using
 *       thread_create()
 */
Thread thread_self(void) {

    uint64_t fs;

    /* Get the contents of the fs register */
    _arch_prctl(ARCH_GET_FS, &fs);

    /* Return thread handle */
    return (Thread)fs;
}
