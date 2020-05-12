//
// Created by MrRen-sdhm on 2020/5/12.
//

#include "../Thread.h"
#include "../CurrentThread.h"

void mysleep(int seconds) {
    timespec t = { seconds, 0 };
    nanosleep(&t, NULL);
}

void threadFunc() {
    printf("tid=%d\n", CurrentThread::tid());
}

void threadFunc2(int x) {
    printf("tid=%d, x=%d\n", CurrentThread::tid(), x);
}

void threadFunc3() {
    printf("tid=%d\n", CurrentThread::tid());
    mysleep(1);
}

class Foo
{
public:
    explicit Foo(double x): x_(x) {}

    void memberFunc() {
        printf("tid=%d, Foo::x_=%f\n", CurrentThread::tid(), x_);
    }

    void memberFunc2(const std::string& text) {
        printf("tid=%d, Foo::x_=%f, text=%s\n", CurrentThread::tid(), x_, text.c_str());
    }

private:
    double x_;
};

int main() {
    printf("pid=%d, tid=%d\n", ::getpid(), CurrentThread::tid());

    // 调用默认构造函数
    Thread t1(threadFunc);
    t1.start();
    printf("t1.tid=%d\n", t1.tid());
    t1.join();

    // 调用自定义构造函数传入线程名，并通过bind传参给线程函数
    Thread t2(std::bind(threadFunc2, 42), "thread for free function with argument");
    t2.start();
    printf("t2.tid=%d\n", t2.tid());
    t2.join();

    // 调用自定义构造函数传入线程名，并传入类成员函数及对象地址，通过bind绑定类成员函数及对象地址
    Foo foo(87.53);
    Thread t3(std::bind(&Foo::memberFunc, &foo), "thread for member function without argument");
    t3.start();
    t3.join();

    // 使用库函数获取对象地址，使用string类构造函数初始化字符串
    Thread t4(std::bind(&Foo::memberFunc2, std::ref(foo), std::string("Test")));
    t4.start();
    t4.join();

    // t1-t4使用了join()，应在之前创建的线程销毁之后再销毁
    // 测试线程的创建与销毁，t5应在t6的创建之前销毁
    {
        Thread t5(threadFunc3, "t5");
        t5.start();
    }
    mysleep(2);
    {
        Thread t6(threadFunc3, "t6");
        t6.start();
        mysleep(2);
    }
    sleep(2);

    // TODO: 获取当前创建的线程数
//    printf("number of created threads %d\n", Thread::numCreated());
}
