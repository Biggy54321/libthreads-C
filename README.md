# libthreads-C

An implementation of **one-one**, **many-many** and **hybrid** user threading
libraries in C.

## Author

**Name:** Bhaskar Pardeshi

**MIS:** 111703041

**Branch:** Computer Engineering

**Batch:** AMPT

**Year:** Third

## Library interface

### Brief

**libthreads-C** consists of three threading libraries, all of which have the same interface (with some minor changes). The three libraries can
be classified depending on how the user threads are mapped to the kernel threads. The three libraries are:

* **One-one** mapped library (Here every user thread is mapped to one and only one kernel thread)
* **Many-many** mapped library (Here a number of user threads are mapped to a number of kernel threads, but there is no relationship between the user thread and the kernel thread as in one-one library)
* **Hybrid** library (Here the user threads can be mapped either in one-one fashion or many-many fashion)

### Types

The library provides the following types to the application program:

| **Type**           | **Description**                                                                           |
|--------------------|-------------------------------------------------------------------------------------------|
| **Thread**         | Thread descriptor (thread handle) which represents the thread in the application program. |
| **ThreadSpinlock** | Thread spin lock used for synchronization                                                 |
| **ThreadMutex**    | Thread mutex used for synchronization                                                     |

All of the types provided by the library are **opaque**, i.e., their implementation is hidden from the user. This is done to prevent any smart IDE or text editor from knowing the implementation of the type and suggesting the members of the type to the application programmer.

### Return status

Most of the functions (which will be discussed below) provided by the library return an integer status, indicating whether the function succeeded or failed. There are two possible cases for functions returning integer:

* If a library function succeeds then it returns **THREAD_SUCCESS** status.
* If a library function fails then it returns **THREAD_FAIL** status. When a library function fails, it sets an error number in order for the application program to know what went wrong. The error number can be fetched using **thread_errno** macro, which returns the value of the thread specific error number. This error number can also be set by the application program for its own use.

### Functions

The classes of functions provided by the library can be broadly divided into three types:

* Thread **control** functions
* Thread **synchronization** functions
* Thread **signal handling** functions

#### Thread control functions

#### Thread create

```
/* Create a user thread */

/* For one-one and many-many */
int thread_create(Thread *thread, void *(*start)(void *), void *arg);

/* For hybrid */
int thread_create(Thread *thread, void *(*start)(void *), void *arg, int type);
```

* This function creates a user thread with the given start function **start** and argument **arg**. For the hybrid library there is an extra parameter indicating the **type** of the user thread being created. To specify the type of thread, the application program must use the provided enumerations viz., **THREAD_TYPE_ONE_ONE** and **THREAD_TYPE_MANY_MANY** for creating the respective mapped thread.
* On success returns **THREAD_SUCCESS**.
* On failure returns **THREAD_FAIL** and sets **thread_errno** to:

    * *EINVAL*: If thread or start argument are invalid
    * *EAGAIN*: If resources cannot be allocated to the thread

#### Thread exit

```
/* Exit from the thread */
void thread_exit(void *ret);
```

* This function when called causes the calling thread to stop its execution. It takes as an argument **ret**, which acts as the return status of the calling thread. This return status can then be fetched by another thread which joins with exited thread.
* The thread can exit either by calling thread\_exit() or by returning from the start function specified in thread\_create().

#### Thread join

```
/* Join with the thread */
int thread_join(Thread thread, void **ret);
```

* This function causes the calling thread to wait for the specified target thread **thread** to finish its execution.
* The return status of the target thread is fetched and stored at the location pointed by the argument **ret**.
* It is a blocking call and will return only after the target thread finishes.
* On success returns **THREAD_SUCCESS**.
* On failure returns **THREAD_FAIL** and sets **thread_errno** to:

    * *EINVAL*: If thread argument is invalid, target thread is already joined or there is another thread already waiting for the target thread
    * *EDEADLK*: If the target and calling thread are same or the target thread is waiting for the calling thread already

#### Thread self

```
/* Get the calling thread descriptor */
Thread thread_self(void);
```

* This function returns the thread descriptor of the calling thread.

#### Thread yield

```
/* Yield the control */
int thread_yield(void);
```

* This function returns the control of the calling thread to the respective scheduler.
* The calling thread is then put to the end of the ready queue until it is again scheduled by the scheduler.
* After rescheduling it returns to the statement after the yield function call.

#### Thread main

```
/* Main thread start function (to be implemented by the user) */
void *thread_main(void *arg);
```

* The actual **main()** function is implemented by the library itself. It is not because the author does not know about **constructor** functions. But it is done so as to provide a uniform treatment to all the threads in the application program. The main() function is a one-one mapped thread by default. Hence in many-many library this poses a problem, also the **Thread-Local-Storage** for the main() function is not set by the library and is done by the compiler, hence the format of the **Thread-Local-Storage**(s) of the library and the compiler are very different. Hence calling any thread library function from the main() routine carelessly may cause the application program to crash.
* Hence to prevent all this the application program using **libthreads-C** will have to implement **thread_main()** function as its own starting
  function.
* The application program will exit directly if the **thread_main()** thread exits.
* This thread is mapped in following ways for the three libraries:

    * **One-one**: **One-one** mapped
    * **Many-many**: **Many-many** mapped
    * **Hybrid**: **One-one** mapped

#### Thread synchronization functions

Synchronization primitives provided by the library include **spinlocks** and **mutexes**. The spinlocks are implemented in all of the three variations of the library, however mutexes are implemented in one-one and many-many library only.

#### Spinlock initialization

```
/* Initialize the spinlock */
int thread_spin_init(ThreadSpinLock *spinlock);
```

* This function initializes the spinlock object pointed by **spinlock** argument.
* The spinlock is initially not owned by any thread.
* On success returns **THREAD_SUCCESS**.
* On failure returns **THREAD_FAIL** and sets **thread_errno** to:

    * *EINVAL*: If spinlock argument is invalid
    * *EAGAIN*: If resources cannot be allocated to create the object

#### Spinlock acquire

```
/* Acquire the spinlock */
int thread_spin_lock(ThreadSpinLock *spinlock);
```

* This function acquires the spinlock pointed by **spinlock** argument.
* The function is a **blocking**, causing the calling thread to be blocked till it does not acquire the spinlock.
* If the spinlock is not acquired then it returns immediately.
* This function causes the waiting thread to perform **busy waiting** instead of going into a wait state. Hence threads waiting on a spinlock will consume CPU resources.
* On success returns **THREAD_SUCCESS**.
* On failure returns **THREAD_FAIL** and sets **thread_errno** to:

    * *EINVAL*: If spinlock argument is invalid

#### Spinlock release

```
/* Release the spinlock */
int thread_spin_unlock(ThreadSpinLock *spinlock);
```

* This function releases the spinlock pointed by **spinlock** argument.
* This function prevents any thread which is not the currently the owner of the spinlock from unlocking it.
* On success returns **THREAD_SUCCESS**.
* On failure returns **THREAD_FAIL** and sets **thread_errno** to:

    * *EINVAL*: If spinlock argument is invalid
    * *EACCES*: If calling thread is not the owner of the spinlock

#### Spinlock destroy

```
/* Deinitialize the spinlock */
int thread_spin_destroy(ThreadSpinLock *spinlock);
```

* This function deallocates the memory which was created at the time of initialization of the spinlock object pointed by **spinlock**.
* On success returns **THREAD_SUCCESS**.
* On failure returns **THREAD_FAIL** and sets **thread_errno** to:

    * *EINVAL*: If spinlock argument is invalid

#### Mutex initialization

```
/* Initialize the mutex */
int thread_mutex_init(ThreadMutex *mutex);
```

* This function initializes the mutex object pointed by **mutex** argument.
* The mutex is initially not owned by any thread.
* On success returns **THREAD_SUCCESS**.
* On failure returns **THREAD_FAIL** and sets **thread_errno** to:

    * *EINVAL*: If mutex argument is invalid
    * *EAGAIN*: If resources cannot be allocated to create the object

#### Mutex acquire

```
/* Acquire the mutex */
int thread_mutex_lock(ThreadMutex *mutex);
```

* This function acquires the mutex pointed by **mutex** argument.
* The function is a **blocking**, causing the calling thread to be blocked till it does not acquire the mutex.
* If the mutex is not acquired then it returns immediately.
* The waiting threads in case of mutexes are made to **go** in **wait state**. Hence the waiting threads will never consume CPU while waiting.
* On success returns **THREAD_SUCCESS**.
* On failure returns **THREAD_FAIL** and sets **thread_errno** to:

    * *EINVAL*: If mutex argument is invalid

#### Mutex release

```
/* Release the mutex */
int thread_mutex_unlock(ThreadMutex *mutex);
```

* This function releases the mutex pointed by **mutex** argument.
* This function prevents any thread which is not the currently the owner of the mutex from unlocking it.
* On success returns **THREAD_SUCCESS**.
* On failure returns **THREAD_FAIL** and sets **thread_errno** to:

    * *EINVAL*: If mutex argument is invalid
    * *EACCES*: If calling thread is not the owner of the mutex

#### Mutex destroy

```
/* Deinitialize the mutex */
int thread_mutex_destroy(ThreadMutex *mutex);
```

* This function deallocates the memory which was created at the time of initialization of the mutex object pointed by **mutex**.
* On success returns **THREAD_SUCCESS**.
* On failure returns **THREAD_FAIL** and sets **thread_errno** to:

    * *EINVAL*: If mutex argument is invalid

#### Thread signal handling functions

#### Thread mask signals

```
/* Mask the specified signals */
int thread_sigmask(int how, sigset_t *set, sigset_t *oldset);
```

* This function can be used to block/unblock a set of signals specified by the argument **set**. The old set of previously blocked signals is stored in the argument **oldset**.
* The action to be performed is specified by the argument **how**, which can take values **SIG_BLOCK**, **SIG_SETMASK** and **SIG_UNBLOCK**.
* If **set** is **NULL** then no operation is performed.
* If **oldset** is **NULL** then the old signal set is not stored.
* Always returns **THREAD_SUCCESS**.
* *NOTE*: The many-many and hybrid libraries use the **SIGALRM** signal internally for scheduling purposes. Hence blocking this signal may cause unexpected behaviour. The library prevents the signal from blocking even if user specifies it. But blocking the **SIGALRM** signal in any external signal handlers would also yield unexpected results.

#### Thread kill

```
/* Send a signal */
int thread_kill(Thread Thread, int signo);
```

* This function sends the signal specified by **signo** to the thread given by the argument **thread**.
* Sending a **SIGALRM** to a thread in case of many-many or hybrid libraries would yield unexpected results
* On success returns **THREAD_SUCCESS**.
* On failure returns **THREAD_FAIL** and sets **thread_errno** to:

    * *EINVAL*: If thread argument is invalid or signal number is invalid

## TODO(s)

* Adding a finer control on the thread **properties** while creating a user thread. This includes setting the **stack size**, **stack guard size** and **priority** for the user thread.
* Changing the **scheduling policy** of **many-many** threads from FIFO to priority based preemptive policy.
* Implementing more **synchronization primitives** such as condition variables, read-write locks and barriers.
* Implementing **thread-specific-storage** which allows the global objects hold data specific to each user thread.
* Implementing **wrappers** for the **system calls**, as using **glibc** wrappers causes the library to crash. Implementing the wrappers compatible with our library will allow us to introduce **thread cancellation** to the library.
* Improving the **error checking** of the library. Currently no error checking is done in the utilities used by the library. The errors are asserted in the utility routines.

## Usage

* The library provides a dynamically linked and statically linked libraries with name **libthread.so** and **libthread.a** respectively. These libraries can be installed using the script given in the **build** directory of each library. The **install.sh** script will install the library in the host system.
* The application program has to include the file **thread.h** and compile the program using **-lthread** option.
* In case of many-many and hybrid threading libraries, the application program will take one command line argument, which would specify the number of kernel threads to be allocated for the many-many user threads. If the user does not specify the argument, then by default **one** kernel thread is allocated for scheduling the many-many threads in both the libraries.
