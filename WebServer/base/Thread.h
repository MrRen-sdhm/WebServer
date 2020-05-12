//
// Created by MrRen-sdhm on 2020/5/12.
//
#pragma once

#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <functional>
#include <memory>
#include <string>

#include "CountDownLatch.h"
#include "NonCopyable.h"

class Thread : noncopyable {
public:
    typedef std::function<void()> ThreadFunc;
    explicit Thread(const ThreadFunc& func, const std::string& name = std::string())
                    : started_(false), joined_(false), pthreadId_(0), tid_(0), func_(func), name_(name), latch_(1) {
        setDefaultName();
    }

    ~Thread() {
        if (started_ && !joined_) pthread_detach(pthreadId_);
        printf("destroy thread: %s tid: %d\n", name_.c_str(), tid_);
    }

    void start();
    int join();
    bool started() const { return started_; }
    pid_t tid() const { return tid_; }
    const std::string& name() const { return name_; }

private:
    void setDefaultName();
    bool started_;
    bool joined_;
    pthread_t pthreadId_;
    pid_t tid_;
    ThreadFunc func_;
    std::string name_;
    CountDownLatch latch_;
};