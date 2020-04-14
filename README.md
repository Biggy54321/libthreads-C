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
* Any thread can join with any other thread.
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

#### Thread equal

```
/* Check if the two thread objects are same */
int thread_equal(Thread thread1, Thread thread2);
```

* As the thread object is an opaque datatype, the user would never know that
  using equality operator would yield appropriate results.
* This function returns zero if the two threads are not equal and one if they are equal. Equal means the two thread objects refer to the same thread.

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

## Todo(s)

* Adding a finer control on the thread **properties** while creating a user thread. This includes setting the **stack size**, **stack guard size** and **priority** for the user thread.
* Changing the **scheduling policy** of **many-many** threads from FIFO to priority based preemptive policy.
* Implementing more **synchronization primitives** such as condition variables, read-write locks and barriers.
* Implementing **thread-specific-storage** which allows the global objects hold data specific to each user thread.
* Implementing **wrappers** for the **system calls**, as using **glibc** wrappers causes the library to crash. Implementing the wrappers compatible with our library will allow us to introduce **thread cancellation** to the library.
* Improving the **error checking** of the library. Currently no error checking is done in the utilities used by the library. The errors are asserted in the utility routines.

## Note

The library can be used on 64 bit systems only, as it uses a hardware feature which is specific to 64 bit systems.

## Usage

* The three libraries are organized into three directories respectively.
* Each directory has three directories:

    * **src**: Which contains the source code
    * **build**: Which contains some scripts required to build the library
    * **bin**: Which contains the compilation binary metadata and results
* There are two scripts in the **build** directory:

    * **install.sh**: Which is used to install the library in the host system
    * **clean.sh**: Which is used to uninstall the library from the host system

* To build the library perform the following steps:

    * Navigate to the **build** directory of the required library.
    * Give executable permissions to both the scripts by the command:

    ```
        $> chmod +x *.sh
    ```

    * Compile and install the library by the command:

    ```
        $> ./install.sh
    ```

    * To uninstall the library run the command:

    ```
        $> ./clean.sh
    ```

* To use the library in the application program:

    * Include the libthreads-C header file as shown below in the application program source code:

    ```
    #include <thread.h>
    ```

    * Implement the the **thread_main()** routine in the application program instead of **main()** routine:

    ```
    void *thread_main(void *arg) {

        /* This function will be the starting function of the application */

        ...

        return NULL;
    }
    ```

    * While compiling the application use the following command:

    ```
    $> gcc <options> <source_files> -lthread
    ```

* While executing the program, in case of **many-many** and **hybrid** libraries, the application will take one command line argument. This argument specifies the **number of kernel threads** to be allocated for scheduling the many-many mapped user threads. If the user does not specify any command line argument then by default the library allocates **one** kernel thread for scheduling the many-many threads in both the libraries.

```
    $> # For one-one library
    $> ./a.out
    $>
    $> # For many-many and hybrid library
    $> ./a.out <number_of_kernel_threads>
```

## Test code

* This repository provides some test codes implementing the library routines.
* This test code is located in the **test** directory.
* The **test** directory has a script named **test.sh** which can be used to test a module of any of the three libraries.
* For testing a module of a library perform the following:

    * Navigate to the **test** directory.
    * Give executable permissions to the script by the command:

    ```
        $> chmod +x ./test.sh
    ```

    * Run the script with help option to get to know the possible options:

    ```
        $> ./test.sh help
    ```

    * The general usage of the script is is given by:

    ```
        $> ./test.sh <library_name> <module_name> <cmd_args>
    ```

    * For example, the following command will run the test code for mutexes in one-one library (for one-one library the third argument is not needed).

    ```
        $> ./test.sh one-one mutex
        Thread mutex testing

        Test 1: Create three threads, where each thread runs for some number
        of iterations. Each thread updates a variable local to itself and each
        of the thread also updates a variable which is shared amongst all.
        However the race on the shared variable is not handled in this case
        Debug: Set cnt, cnt1, cnt2 and cnt3 all to zero
        Debug: thread_main() created three threads
        Debug: thread_main() called join on the threads
        Debug: cnt = 122265
        Debug: cnt1 = 100000
        Debug: cnt2 = 100000
        Debug: cnt3 = 100000
        Debug: The globals are not consistent
        Test 1: Succeeded

        Test 2: Create three threads, where each thread runs for some number
        of iterations. Each thread updates a variable local to itself and
        each of the thread also updates a variable which is shared amongst
        all. The race on the shared variable is handled in this case using
        mutexes
        Debug: Set cnt, cnt1, cnt2 and cnt3 all to zero
        Debug: thread_main() initialized mutex
        Debug: thread_main() created three threads
        Debug: thread_main() called join on the threads
        Debug: thread_main() deinitialized mutex
        Debug: cnt = 300000
        Debug: cnt1 = 100000
        Debug: cnt2 = 100000
        Debug: cnt3 = 100000
        Debug: The globals are consistent
        Test 2: Succeeded
    ```

    * Another example for many-many library can be to test the signal handling functions with four kernel threads.

    ```
        $> ./test.sh many-many signal 4
        Thread signal handling testing

        Test 1: Sending a signal to an infinitely sleeping thread
        Debug: thread_main() created thread1()
        Debug: In thread1(), waiting for signal
        Debug: thread_main() sent SIGUSR1 to thread1()
        Debug: thread_main() called join on thread1()
        Debug: In SIGUSR1 handler, exiting from the thread
        Test 1: Succeeded

        Test 2: Blocking a signal in a thread whose handler may cause infinite wait of the target thread
        Debug: thread_main() created thread2()
        Debug: In thread2(), sending SIGUSR2 to thread_main()
        Debug: thread_main() did not receive SIGUSR2 sent by thread2()
        Test 2: Succeeded
    ```

    * Sometimes the debug prints can be very irritating, hence to block all the debug prints in the test code output, open the **test.sh** file and uncomment the line **GCC_COMPILATION_FLAGS="-DBLOCK_DEBUG_PRINTS"**, which will block all the prints with **Debug:** prefix.
