#include "./mods/utils.h"
#include "./mods/lock.h"
#include "./mods/sig.h"
#include "./hthread_priv.h"
#include "./hthread_sig.h"

/**
 * @brief Update the signal mask
 *
 * Changes the current signal mask of thread
 *
 * @param[in] how What action to perform
 * @param[in] set Pointer to the signal set to be worked upon
 * @param[out] oldset Pointer to the signal set which will store the old signal
 *             mask. Will store the previously block signal set
 */
void hthread_sigmask(int how, sigset_t *set, sigset_t *oldset) {

    /* Check for errors */
    assert(set);
    assert(oldset);

    /* Remove SIGALRM from the signal set (just becuz i use it) */
    sigdelset(set, SIGALRM);

    /* Call the signal process mask function */
    sigprocmask(how, set, oldset);
}

/**
 * @brief Kill a thread
 *
 * Send a signal to the target thread
 *
 * @param[in] hthread Target thread handle
 * @param[in] sig_num Signal number
 */
void hthread_kill(HThread hthread, int sig_num) {

    Signal *sig;

    /* Check for errors */
    assert(hthread);

    /* Wait till the target thread is not properly initialized */
    while (hthread->state == HTHREAD_STATE_INIT);

    /* Depending on the target thread type */
    switch (hthread->type) {

        case HTHREAD_TYPE_ONE_ONE:

            /* Send the signal to the thread */
            sig_send(ONE_TLS(hthread)->tid, sig_num);
            break;

        case HTHREAD_TYPE_MANY_MANY:

            /* Create a new signal */
            sig = alloc_mem(Signal);

            /* Initialize the signal */
            sig->sig = sig_num;

            /* Lock the signal list */
            lock_acquire(&MANY_TLS(hthread)->sig_lock);

            /* Add the signal to the thread's list of deliverables */
            list_enqueue(&MANY_TLS(hthread)->sig_list, sig, sig_mem);

            /* Unlock the signal list */
            lock_release(&MANY_TLS(hthread)->sig_lock);
            break;

        default:
            break;
    }
}
