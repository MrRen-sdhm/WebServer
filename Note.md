## 1、WebServer关键技术及模型

### EventLoop—事件循环

- EventLoop是不可拷贝的
- 每个线程只能有一个事件循环，EventLoop构造函数需要检测当前线程是否创建了其他EventLoop对象
- EventLoop的构造函数需要记住本对象所属的线程ID
- EventLoop对象由IO线程创建，其寿命同所属线程一样长
- 事件循环必须在IO线程执行，需要提供接口检查这一条件，并在loop中进行检测



## 2、pthread线程编程接口相关

### __thread关键字

\_\_thread是GCC内置的线程局部存储设施，存取效率可以和全局变量相比。\_\_thread变量每一个线程有一份独立实体，各个线程的值互不干扰。可以用来修饰那些带有全局性且值可能变，但是又不值得用全局变量保护的变量。

```C++
#include<iostream>  
#include<pthread.h>  
#include<unistd.h>  
using namespace std;  
__thread int var=5;
void* worker1(void* arg);  
void* worker2(void* arg);  
int main(){  
    pthread_t pid1,pid2;  
    static __thread int temp=10; // 修饰函数内的static变量
    pthread_create(&pid1, NULL, worker1, NULL);
    pthread_create(&pid2, NULL, worker2, NULL);
    pthread_join(pid1, NULL);
    pthread_join(pid2, NULL);
    cout << temp << endl; // 输出10
    return 0;
}  
void* worker1(void* arg){
    cout << ++var << endl; // 输出 6
}  
void* worker2(void* arg){
    sleep(1); // 等待线程1改变var值，验证是否影响线程2
    cout << ++var << endl; // 输出6
}  
```

```
输出
6
6         // 可见__thread值线程间互不干扰
10
```



## 3、Unix网络编程接口相关

#### INADDR_ANY

所有32位均为0 的地址是IPv4 的未指明地址(unspecified address) 。这个IP地址只能作为源
地址出现在IPv4分组中，而目是在其发送主机处于获悉 自 身IP地址之前的自举引导过程期间。
在套接字API中该地址称为通配地址，其通常为人所知的名字是 INADDR_ANY。在套接字API中
绑定该地址（例如为了监听某套接字）表示会接受目的地为任何节点的IPv4地址的客户连接。

> 摘自UNP 附录A.4.3 未指明地址