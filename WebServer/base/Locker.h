//
// Created by MrRen-sdhm on 2020/5/12.
//
#pragma once

#include <pthread.h>
#include <cstdio>
#include <errno.h>
#include <time.h>

#include "NonCopyable.h"

/**
 * 互斥锁封装类
 */
class MutexLock : noncopyable {
public:
    MutexLock() { pthread_mutex_init(&mutex, NULL); }
    ~MutexLock() {
        pthread_mutex_lock(&mutex);
        pthread_mutex_destroy(&mutex);
    }
    void lock() { pthread_mutex_lock(&mutex); }
    void unlock() { pthread_mutex_unlock(&mutex); }
    pthread_mutex_t *get() { return &mutex; }

private:
    pthread_mutex_t mutex;

private:
    friend class Condition; // 友元类不受访问权限影响
};

/**
 * 使用RAII机制的互斥锁
 */
class MutexLockGuard : noncopyable {
public:
    explicit MutexLockGuard(MutexLock &_mutex) : mutex(_mutex) { mutex.lock(); }
    ~MutexLockGuard() { mutex.unlock(); }

private:
    MutexLock &mutex;
};


/**
 * 条件变量封装类
 */
class Condition : noncopyable {
public:
    explicit Condition(MutexLock &_mutex) : mutex(_mutex) {
        pthread_cond_init(&cond, NULL);
    }
    ~Condition() { pthread_cond_destroy(&cond); }
    void wait() { pthread_cond_wait(&cond, mutex.get()); }
    void notify() { pthread_cond_signal(&cond); }
    void notifyAll() { pthread_cond_broadcast(&cond); }
    bool waitForSeconds(int seconds) {
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME, &abstime);
        abstime.tv_sec += static_cast<time_t>(seconds);
        return ETIMEDOUT == pthread_cond_timedwait(&cond, mutex.get(), &abstime);
    }

private:
    MutexLock &mutex;
    pthread_cond_t cond;
};
