#include <assert.h>
#include <stddef.h>

#include "./sigmask.h"

/**
 * @brief Block all the signals
 *
 * Sets a mask so as to block all the signals
 */
void sigmask_block_all(void) {

    sigset_t mask;

    /* Set all the signals in the mask */
    sigfillset(&mask);

    /* Set the mask */
    sigprocmask(SIG_BLOCK, &mask, NULL);
}

/**
 * @brief Unblock all the signals
 *
 * Sets a mask so as to unblock all the signals
 */
void sigmask_unblock_all(void) {

    sigset_t mask;

    /* Set all the signals in the mask */
    sigfillset(&mask);

    /* Set the mask */
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

/**
 * @brief Blocks only the specified set of signals
 *
 * Blocks the signals which are set in the given signal set (for setting and
 * unsetting refer the man page)
 *
 * @param[in] mask Pointer to the signal set
 */
void sigmask_block(sigset_t *mask) {

    /* Check for errors */
    assert(mask);

    /* Unblock all the signals */
    sigmask_unblock_all();

    /* Block the requested signal set */
    sigprocmask(SIG_BLOCK, mask, NULL);
}

/**
 * @brief Unblocks only the specified set of signals
 *
 * Unblocks the signals which are set in the given signal set (for setting and
 * unsetting refer the man page)
 *
 * @param[in] mask Pointer to the signal set
 */
void sigmask_unblock(sigset_t *mask) {

    /* Check for errors */
    assert(mask);

    /* Block all the signals */
    sigmask_block_all();

    /* Unblock the requested signals */
    sigprocmask(SIG_UNBLOCK, mask, NULL);
}

/**
 * @brief Get current mask
 *
 * Sets the current signal set which are blocked
 *
 * @param[out] mask Pointer to the signal set
 */
void sigmask_get_mask(sigset_t *mask) {

    /* Check for errors */
    assert(mask);

    /* Get the current mask */
    sigprocmask(0, NULL, mask);
}