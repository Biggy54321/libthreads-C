# Hybrid user threading model

**libthreads-C** is a user level threading library which can be used on 64 bit
systems only. The following functionalities are provided by the hybrid
threading model of the library.

## Thread control

### Thread library initialization

As the threading model is hybrid, some of the user threads may be of type
**many-many**. Hence multiple user threads can be mapped to any of the
available kernel threads. In order the set the number of kernel threads for
scheduling the many-many user threads, the application program can do as follows

```
/* Set the number of kernel threads required */
int nb_kernel_threads = 5;

/* Call the library function to initialize the kernel threads */
hthread_init(nb_kernel_threads);
```

What this function does is create the specified number of kernel threads, and
makes the many-many scheduler run of each of them. The scheduler then looks for
the many-many threads to be scheduled in the global list of many-many user
threads, if found it removes a thread from the list and schedules it. The user
threads are **peemptively multitasked** and are scheduled in a FIFO manner.
Hence each user thread gets some time on an available kernel thread. The user
thread is not bound to a specific kernel thread and may get scheduled on
different kernel thread in different scheduling turns. This function should be
called before using any other function of the library, as it may initialize
any other global data structures required by the library.

### Thread create
Two types of threads can be created using the library - **one-one** or
**many-many**. Each thread must have a start routine with some argument. The
signature of the start routine is fixed also the type of argument is also fixed.
The application program can create the thread as follows

```
/* Thread start function */
void *thread_start(void *arg) {

    ...
}

/* Main thread */
void main() {

    ...

    HThread hth;
    int arg = 5;

    /* Create a one one thread */
    hth = hthread_create(thread_start, &arg, HTHREAD_TYPE_ONE_ONE);

    ...
}
```

The thread create routine returns a structure called the **thread handle** which
can be used to perform operations on the thread. To pass compound arguments, one
can create a structure and pass the address of the structure instance. For
creating many-many thread, the application program must initialize the kernel
threads using the previously discussed library function. After that the same
thread create routine can be used, however the third argument should be
**HTHREAD\_TYPE\_MANY\_MANY**. When thread create is called, some resources
are allocated for the thread.

### Thread exit

A user thread can either return naturally from it's start function or using
the exit library routine. The thread can return value when it exits using the
library function too. The exit routine can be called from any level of depth
in a particular thread. The exit routine is as follows

```
/* A function called by the thread start function */
void inner_func(void) {

    int ret = 5;

    /* Exit from the thread */
    hthread_exit((void *)ret);
}

/* Thread start function */
void *thread_start(void *arg) {

    ...

    if (...) {

        /* Call another function */
        inner_func();
    }

    /* Natural return value */
    return (void *)1;
}
```

In the above example, if the thread satisfies the predicate, then it calls
the inner\_func(). The inner\_func makes a exit routine call with return value
of 5. However if the predicate would have been false, then the return value of
the thread would be 1 rather than 5. This return value is available for any
other thread which waits for the current thread to finish. The resources
allocated to the thread are not released when exit is called.

### Thread join

The value returned by the thread by a natural return or exit routine call, can
be obtained by any other thread. Any thread can wait for the completion of any
another thread. This is called as joining. When the thread waits for a **target
thread**, there may be two cases - the target thread is running or the target
thread is finished. If the target thread is running then the **calling thread**
will wait for its execution to get completed. If the target thread is finished
then the calling thread does not wait. After the wait is over, the calling
thread fetches the return value of the target thread from its **Thread Local
Storage**. If many calling threads are waiting on a target thread for joining,
then only one calling thread will continue its execution ahead. Rest all the
threads will hence wait infinitely. The join routine can be used as follows

```
/* Thread start function */
void *start_routine(void *arg) {

    ...

    return (void *)5;
}

/* Main thread */
void main() {

    ...

    /* Container for return value */
    void *ret;

    /* Create a one-one thread */
    HThread hth = hthread_create(start_routine, NULL, HTHREAD_TYPE_ONE_ONE);

    /* Wait for the thread to finish */
    hthread_join(hth, &ret);

    ...
}
```

In the above example, the main thread waits for the one-one thread, and after
its completion gets the return value of 5 in its respective variable. Using
this function, even a one-one thread can wait for many-many thread and
vice-versa. The resources allocated to the target thread is freed by the caller
after a successful join. If a join is performed on a target thread which has
already finished execution, then the caller thread will wait infinitely. Also
if the caller thread performs a join on a target thread which is waiting
infinitely, then it will also wait infinitely.

### Thread self

It may happen that a particular thread may want to perform some operations on
itself. For this the current thread must know its thread handle. To get the
thread handle of itself, a thread may use the self routine of the library as
follows

```
/* Thread start routine */
void *start_routine(void *arg) {

    HThread hth;

    /* Get the thread handle */
    hth = hthread_self();

    ...
}
```

This routine can be called anywhere inside the thread and at any depth. It is
guranteed that the value returned by the function will be the same throughout
different calls.

### Thread library deinitialization

After the application program is done using the thread library and still wants
to something else which may not require the current library function, it may
call the thread library deinitialization routine, to stop all the kernel
threads and deallocate any resources previously allocated. For this simply
do as follows

```
void main() {
    ...
    hthread_deinit();
    ...
}
```

## Thread synchronization

### Thread mutexes

For **synchronization** mutex locks are provided. The user will have to create
a lock instance, initialize it and then lock or unlock as and when required.
The following example illustrates the usage of locks

```
/* Shared variable */
int g_i = 0;
/* Create a mutex and initialize it using the macro defined by the library */
HThreadMutex mut = HTHREAD_MUTEX_INITIALIZER;

void *thread1(void *arg) {

    /* Acquire the lock */
    hthread_mutex_lock(&mut);

    printf("%d\n", g_i++);

    /* Release the lock */
    hthread_mutex_unlock(&mut);
}

void *thread2(void *arg) {

    /* Acquire the lock */
    hthread_mutex_lock(&mut);

    printf("%d\n", g_i++);

    /* Release the lock */
    hthread_mutex_unlock(&mut);
}
```

In the above example, the result will have two number - 0 and 1 on two seperate
lines. It is because the access to the variable was protected using the lock.
The lock ensured that the variable was not being read and written simultaneously
by the two threads. Only one thread will enter the critical section being
protected by the locks, while rest all the threads will be waiting on the
lock to be released.

## Thread signal handling

### Thread signal mask

Each thread can block a particular set of signals. The set of signals blocked
by the thread are local to the thread and not process-wide. The set of signals,
can be blocked or unblocked using the sigmask routine of the library, as follows

```
/* To unblock a signal set */
hthread_sigmask(SIG_UNBLOCK, &set, NULL);

/* To add signals to the blocked set */
hthread_sigmask(SIG_BLOCK, &set, NULL);

/* To update the blocked signals */
hthread_sigmask(SIG_SETMASK, &set, NULL);
```

The third argument is the old set of blocked signals, and is returned if a
non-null argument is passed to it.

### Thread kill

A signal can be sent to a target thread using the kill routine of the library.
To direct a signal to the thread its thread handle is required.

```
hthread_kill(target_hth, SIGINT);
```

The above example will send SIGINT to the thread with the passed handle.

## Extras

* The library makes use of **SIGALRM** signal for scheduling the many-many user
  threads. (Any signal can be used, but I used SIGALRM because it sounds good).
  As it is used, it cannot be used the the application program and **should not
  be masked** by any means in the application program. Hence to update the
  signal mask one should only use the functions provided by the library which
  take care of this situation.

* The library still does not handle the **deallocation** of the user-threads
  properly. The join function requires the TLS of the thread to be freed
  partially. Hence this must be improved.

* During synchronization and joining, the spinning threads are **not** made to
  go in **wait state**. This should be improved to improve the efficiency of
  the library.

* The signal delivery in case of many-many user threads causes the user thread
  to remain on the same kernel thread till all of the delivered signals dont
  remain pending. Hence if the signals remain pending infinitely then it may
  result in one kernel thread being used only for one user threads.

* The TLS of the main thread is not being set currently. Hence many operations
  cannot be performed on the main thread.
