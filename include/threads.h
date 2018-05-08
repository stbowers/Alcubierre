/*
 * Created by Sean Bowers
 * CS 2060 section 3, Spring 2018
 * University of Colorado at Colorado Springs
 *
 * Licensed under the MIT License (see LICENSE.txt)
 */
/* Defines cross-platform threading functions
 */

#ifndef __THREADS_H__
#define __THREADS_H__

#ifdef __UNIX__
#include <pthread.h>

// macOS does not implement pthread barriers correctly, so this is a re-implementation using other pthread features
// code from http://blog.albertarmea.com/post/47089939939/using-pthreadbarrier-on-mac-os-x
#ifdef __MACOS__

#include <errno.h>

typedef int pthread_barrierattr_t;
typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int tripCount;
} pthread_barrier_t;


static int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned int count)
{
    if(count == 0)
    {
        errno = EINVAL;
        return -1;
    }
    if(pthread_mutex_init(&barrier->mutex, 0) < 0)
    {
        return -1;
    }
    if(pthread_cond_init(&barrier->cond, 0) < 0)
    {
        pthread_mutex_destroy(&barrier->mutex);
        return -1;
    }
    barrier->tripCount = count;
    barrier->count = 0;
    
    return 0;
}

static int pthread_barrier_destroy(pthread_barrier_t *barrier)
{
    pthread_cond_destroy(&barrier->cond);
    pthread_mutex_destroy(&barrier->mutex);
    return 0;
}

static int pthread_barrier_wait(pthread_barrier_t *barrier)
{
    pthread_mutex_lock(&barrier->mutex);
    ++(barrier->count);
    if(barrier->count >= barrier->tripCount)
    {
        barrier->count = 0;
        pthread_cond_broadcast(&barrier->cond);
        pthread_mutex_unlock(&barrier->mutex);
        return 1;
    }
    else
    {
        pthread_cond_wait(&barrier->cond, &(barrier->mutex));
        pthread_mutex_unlock(&barrier->mutex);
        return 0;
    }
}
#endif // __MACOS__

#elif __WIN32__
#include <windows.h>
#endif
#include <engine.h>

/* Data types defined (typdef to system implementation) by this header
 * Thread_t - a thread handle
 * ThreadProcess_t - the signature of the funciton to be passed to createThread
 * ThreadLock_t - some sort of object that can be locked for access protection (mutex-like behaviour)
 * ThreadCondition_t - a conditional variable for threads to wait for a signal regarding a condition
 * ThreadBarrier_t - blocks a thread until a certain number of thread have reached the barrier (used to sync threads)
 */

#ifdef __UNIX__
typedef pthread_t Thread_t;
typedef void* (*ThreadProcess_t)(void*);
typedef pthread_mutex_t ThreadLock_t;
typedef pthread_cond_t ThreadCondition_t;
typedef pthread_barrier_t ThreadBarrier_t;
#elif __WIN32__
typedef HANDLE Thread_t;
typedef LPTHREAD_START_ROUTINE ThreadProcess_t;
// critical sections have a few advantages over mutexes when used over only one process
typedef CRITICAL_SECTION ThreadLock_t;
typedef CONDITION_VARIABLE ThreadCondition_t;
typedef SYNCHRONIZATION_BARRIER ThreadBarrier_t;
#endif

/* Functions defined by this header (as macros to the system functions)
 * createThread(Thread_t* handle, void* (*threadFunction)(void*), void* data)
 * createLock(Lock_t* lock)
 * createConditionVariable(ThreadCondition_t* condition)
 * createBarrier(ThreadBarrier* barrier, int numThreads)
 * 
 * lockThreadLock(ThreadLock_t* lock)
 * unlockThreadLock(ThreadLock_t* lock)
 * 
 * waitForConditionSignal(ThreadCondition_t* condition, ThreadLock_t* lock)
 * sendConditionSignal(ThreadCondition_t* condition) // only wakes one thread
 * broadcastConditionSignal(ThreadCondition_t* condition) // wakes all threads waiting on signal
 * 
 * enterThreadBarrier(ThreadBarrier_t* barrier)
 * 
 * exitThread(int returnCode)
 * joinThread(Thread_t* handle)
 */

#ifdef __UNIX__
#define createThread(handle, function, data)\
    pthread_create(handle, NULL, function, data)

#define createLock(handle)\
    pthread_mutex_init(handle, NULL)

#define createConditionVariable(condition)\
    pthread_cond_init(condition, NULL)

#define createBarrier(barrier, numThreads)\
    pthread_barrier_init(barrier, NULL, numThreads)

/* Thread lock macros */
#define lockThreadLock(lock)\
    pthread_mutex_lock(lock)

#define unlockThreadLock(lock)\
    pthread_mutex_unlock(lock)

/* Condition variable macros */
#define waitForConditionSignal(condition, lock)\
    pthread_cond_wait(condition, lock)

#define sendConditionSignal(condition)\
    pthread_cond_signal(condition)

#define broadcastConditionSignal(condition)\
    pthread_cond_broadcast(condition)

/* Barrier macros */
#define enterThreadBarrier(barrier)\
    pthread_barrier_wait(barrier)

// Exit current thread
#define exitThread(code)\
    pthread_exit(code)

// Join thread
#define joinThread(handle)\
    pthread_join(*handle, NULL)
#elif __WIN32__
#define createThread(handle, function, data)\
    *handle=CreateThread(NULL, 0, function, data, 0, NULL)

#define createLock(handle)\
    InitializeCriticalSection(handle)

#define createConditionVariable(condition)\
    InitializeConditionVariable(condition)

// spin count is always 0 to make functionality similar to pthreads barriers
#define createBarrier(barrier, numThreads)\
    InitializeSynchronizationBarrier(barrier, numThreads, 0)

/* Thread lock macros */
#define lockThreadLock(lock)\
    EnterCriticalSection(lock)

#define unlockThreadLock(lock)\
    LeaveCriticalSection(lock)

/* Condition variable macros */
#define waitForConditionSignal(condition, lock)\
    SleepConditionVariableCS(condition, lock, INFINITE)

#define sendConditionSignal(condition)\
    WakeConditionVariable(condition)

#define broadcastConditionSignal(condition)\
    WakeAllConditionVariable(condition)

/* Barrier macros */
#define enterThreadBarrier(barrier)\
    EnterSynchronizationBarrier(barrier, 0)

// Exit the current thread with the given code
#define exitThread(code)\
    ExitThread(code)

// Wait for the given thread to end
#define joinThread(handle)\
	WaitForSingleObject(*handle, INFINITE)
#endif

#endif //__THREADS_H__
