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

* Thread control functions

* Thread synchronization functions

* Thread signal handling functions

#### Thread control functions

```
/* Create a user thread */

/* For one-one and many-many */
int thread_create(Thread *thread, void *(*start)(void *), void *arg);

/* For hybrid */
int thread_create(Thread *thread, void *(*start)(void *), void *arg, int type);
```
* **Thread create**

    + This function creates a user thread with the given start function **start** and argument **arg**. For the hybrid library there is an extra parameter indicating the **type** of the user thread being created. To specify the type of thread, the application program must use the provided enumerations viz., **THREAD_TYPE_ONE_ONE** and **THREAD_TYPE_MANY_MANY** for creating the respective mapped thread.
    + On success returns **THREAD_SUCCESS**.
    + On failure returns **THREAD_FAIL** and sets **thread_errno** to:

        * *EINVAL*: If thread or start argument are invalid
        * *EAGAIN*: If resources cannot be allocated to the thread



#### Thread synchronization functions

#### Thread signal handling functions

