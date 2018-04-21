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
typedef pthread_cond_t ThreadCondition_t
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
    pthread_create(handle, &threadAttributes, function, data)
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