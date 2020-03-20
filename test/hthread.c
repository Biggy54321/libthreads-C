#include "./hthread.h"
#include "./hthread_list.h"
#include "./hthread_kernel.h"

/**
 * @brief Initialize the hybrid thread library
 *
 * Initializes all the required globals. Creates the kernel threads required
 * for the library to schedule the many-many threads
 *
 * @param[in] nb_kthds Number of kernel threads many-many part
 */
void hthread_init(int nb_kthds) {

    /* Initialize the thread list */
    hthread_list_init();

    /* Initialize the kernel threads */
    hthread_kernel_threads_init(nb_kthds);
}
































HThread hthread_create(void *(*start)(void *), void *arg, HThreadType type);

void hthread_join(HThread hthread, void **ret);

void hthread_exit(void *ret);

HThread hthread_self(void);

void hthread_mutex_init(HThreadMutex *mutex);

void hthread_mutex_lock(HThreadMutex *mutex);

void hthread_mutex_unlock(HThreadMutex *mutex);

void hthread_deinit(void);

