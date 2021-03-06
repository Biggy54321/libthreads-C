#include "./mods/lock.h"
#include "./mmrll.h"
#include "./mmsched.h"
#include "./thread.h"
#include "./thread_descr.h"

/* Get the global user thread id */
extern int nxt_utid;
/* Get the global user thrad id lock */
extern Lock nxt_utid_lk;

/* Default number of kernel threads */
#define DEFAULT_NB_KTHREADS  (1u)

/**
 * @brief The main function for the application program that will use the
 *        library
 *
 * In order to generalize the thread concept the main thread is implemented
 * by the library and the application program has to implement the thread_main()
 * routine as a starting thread. This routine will be a one-one mapped thread
 *
 * @param[in] argc Number of command line arguments
 * @param[in] argv Array of command line arguments
 * @return Integer
 */
int main(int argc, char *argv[]) {

    Thread main_td;
    int nb_kthreads;

    /* If the number of command line arguments are less than two */
    if (argc < 2) {

        /* Use the default number of kernel threads */
        nb_kthreads = DEFAULT_NB_KTHREADS;
    } else {

        /* Else use the given number of kernel threads */
        nb_kthreads = atoi(argv[1]);
    }

    /* Initialize the global user thread id */
    nxt_utid = 0;

    /* Initialize the global use thread id lock */
    lock_init(&nxt_utid_lk);

    /* Initialize the many-many ready list */
    mmrll_init();

    /* Initialize the schedulers */
    mmsched_init(nb_kthreads);

    /* Create the main thread */
    thread_create(&main_td, thread_main, NULL, THREAD_TYPE_ONE_ONE);

    /* Wait for its completion */
    while (!td_is_over(main_td));

    /* If the main thread is not joined */
    if (!td_is_joined(main_td)) {

        /* Free the main thread descriptor */
        td_oo_free(main_td);
    }

    /* Deinitialize the schedulers */
    mmsched_deinit();

    return 0;
}
