#include "./thread.h"
#include "./thread_descr.h"

/**
 * @brief The main function for the application program that will use the
 *        library
 *
 * In order to generalize the thread descriptors for every thread in the
 * application program the main function is implemented by the library itself.
 * The application program has to implement the thread_main() function as
 * the start thread for itself. (As the thread descriptor of the actual main
 * thread is different as compared to what is used by the library)
 *
 * @return Integer
 */
int main(void) {

    Thread main_td;

    /* Create the main thread */
    thread_create(&main_td, thread_main, NULL);

    /* Wait for its completion */
    while (!td_is_over(main_td));

    /* If the main thread is not joined */
    if (!td_is_joined(main_td)) {

        /* Free the main thread descriptor */
        td_free(main_td);
    }

    return 0;
}
