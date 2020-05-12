//
// Created by MrRen-sdhm on 2020/5/12.
//
#pragma once

class noncopyable {
protected:
    noncopyable() {}
    ~noncopyable() {}

private:
    noncopyable(const noncopyable&);
    const noncopyable& operator=(const noncopyable&);
};