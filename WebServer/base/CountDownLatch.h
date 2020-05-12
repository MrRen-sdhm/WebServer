//
// Created by MrRen-sdhm on 2020/5/12.
//
#pragma once

#include "Locker.h"
#include "NonCopyable.h"

/**
 * CountDownLatch的主要作用是确保Thread中传进去的func真的启动了以后，外层的start才返回
 */
class CountDownLatch : noncopyable {
public:
    explicit CountDownLatch(int count);
    void wait();
    void countDown();

private:
    mutable MutexLock mutex_;
    Condition condition_;
    int count_;
};