/**
 * @file thread_lock.h
 * @brief 一个进程内的多个线程之间的互斥机制
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_THREAD_LOCk_H_
#define BASE_INC_THREAD_LOCk_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_ThreadLock 线程互斥机制
 * @{
 */

#if (defined(__linux__))
#include <pthread.h>
#elif (defined(_WIN32) || defined(_WIN64))
#include <Windows.h>
#endif

#include "misc_util.h"
#include "str_util.h"

struct ThreadMutex
{
    DISALLOW_COPY_AND_ASSIGN(ThreadMutex);

public:
    ThreadMutex()
    {
#if (defined(__linux__))
        pthread_mutexattr_t mattr;
        pthread_mutexattr_init(&mattr);
        pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_PRIVATE);
        pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_TIMED_NP);
        pthread_mutex_init(&mutex_, &mattr);
        pthread_mutexattr_destroy(&mattr);
#elif (defined(_WIN32) || defined(_WIN64))
        ::InitializeCriticalSection(&mutex_);
#endif
    }

    ~ThreadMutex()
    {
#if (defined(__linux__))
        pthread_mutex_destroy(&mutex_);
#elif (defined(_WIN32) || defined(_WIN64))
        ::DeleteCriticalSection(&mutex_);
#endif
    }

    void Lock()
    {
#if (defined(__linux__))
        pthread_mutex_lock(&mutex_);
#elif (defined(_WIN32) || defined(_WIN64))
        ::EnterCriticalSection(&mutex_);
#endif
    }

    void Unlock()
    {
#if (defined(__linux__))
        pthread_mutex_unlock(&mutex_);
#elif (defined(_WIN32) || defined(_WIN64))
        ::LeaveCriticalSection(&mutex_);
#endif
    }

    /**
     * @brief 尝试加锁
     * @return true: 加锁成功 false: 加锁失败
     */
    bool TryLock()
    {
#if (defined(__linux__))
        return (0 == pthread_mutex_trylock(&mutex_));
#elif (defined(_WIN32) || defined(_WIN64))
        return (TRUE == ::TryEnterCriticalSection(&mutex_));
#endif
    }

private:
#if (defined(__linux__))
    pthread_mutex_t mutex_;
#elif (defined(_WIN32) || defined(_WIN64))
    CRITICAL_SECTION mutex_;
#endif
};

template<typename MutexType>
class AutoThreadMutex
{
    DISALLOW_COPY_AND_ASSIGN(AutoThreadMutex);

public:
    explicit AutoThreadMutex(MutexType* mutex)
    {
        mutex_ = mutex;

        if (mutex_ != NULL)
        {
            mutex_->Lock();
        }
    }

    ~AutoThreadMutex()
    {
        if (mutex_ != NULL)
        {
            mutex_->Unlock();
            mutex_ = NULL;
        }
    }

private:
    MutexType* mutex_;
};

struct ThreadRWLock
{
    DISALLOW_COPY_AND_ASSIGN(ThreadRWLock);

public:
    ThreadRWLock()
    {
        pthread_rwlock_init(&rwlock_, NULL);
    }

    ~ThreadRWLock()
    {
        pthread_rwlock_destroy(&rwlock_);
    }

    void WLock()
    {
        pthread_rwlock_wrlock(&rwlock_);
    }

    void RLock()
    {
        pthread_rwlock_rdlock(&rwlock_);
    }

    void Unlock()
    {
        pthread_rwlock_unlock(&rwlock_);
    }

    /**
     * @brief 尝试加写锁
     * @return true: 加锁成功 false: 加锁失败
     */
    bool WTryLock()
    {
        return (0 == pthread_rwlock_trywrlock(&rwlock_));
    }


    /**
     * @brief 尝试加读锁
     * @return true: 加锁成功 false: 加锁失败
     */
    bool RTryLock()
    {
        return (0 == pthread_rwlock_tryrdlock(&rwlock_));
    }

private:
    pthread_rwlock_t rwlock_;
};

template<typename RWLockType>
class AutoThreadWLock
{
    DISALLOW_COPY_AND_ASSIGN(AutoThreadWLock);

public:
    explicit AutoThreadWLock(RWLockType* rwlock)
    {
        rwlock_ = rwlock;

        if (rwlock_ != NULL)
        {
            rwlock_->WLock();
        }
    }

    ~AutoThreadWLock()
    {
        if (rwlock_ != NULL)
        {
            rwlock_->Unlock();
            rwlock_ = NULL;
        }
    }

private:
    RWLockType* rwlock_;
};

template<typename RWLockType>
class AutoThreadRLock
{
    DISALLOW_COPY_AND_ASSIGN(AutoThreadRLock);

public:
    explicit AutoThreadRLock(RWLockType* rwlock)
    {
        rwlock_ = rwlock;

        if (rwlock_ != NULL)
        {
            rwlock_->RLock();
        }
    }

    ~AutoThreadRLock()
    {
        if (rwlock_ != NULL)
        {
            rwlock_->Unlock();
            rwlock_ = NULL;
        }
    }

private:
    RWLockType* rwlock_;
};

#define SCOPE_GUARD_LINE_NAME(name, line) TOKEN_CAT(name, line)

#if __cplusplus >= 201103L // c++11
#define AUTO_THREAD_MUTEX(mutex) AutoThreadMutex<decltype(mutex)> SCOPE_GUARD_LINE_NAME(mutex_, __LINE__)(&mutex)
#define AUTO_THREAD_WLOCK(rwlock) AutoThreadWLock<decltype(rwlock)> SCOPE_GUARD_LINE_NAME(rwlock_, __LINE__)(&rwlock)
#define AUTO_THREAD_RLOCK(rwlock) AutoThreadRLock<decltype(rwlock)> SCOPE_GUARD_LINE_NAME(rwlock_, __LINE__)(&rwlock)
#else
#define AUTO_THREAD_MUTEX(mutex) AutoThreadMutex<typeof(mutex)> SCOPE_GUARD_LINE_NAME(mutex_, __LINE__)(&mutex)
#define AUTO_THREAD_WLOCK(rwlock) AutoThreadWLock<typeof(rwlock)> SCOPE_GUARD_LINE_NAME(rwlock_, __LINE__)(&rwlock)
#define AUTO_THREAD_RLOCK(rwlock) AutoThreadRLock<typeof(rwlock)> SCOPE_GUARD_LINE_NAME(rwlock_, __LINE__)(&rwlock)
#endif // __cplusplus >= 201103L

/** @} Module_ThreadLock */
/** @} Module_Base */

#endif // BASE_INC_THREAD_LOCk_H_
