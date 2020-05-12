#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>

#include "threadpool.h"
#include "http_conn.h"

#define DEFUALT_PORT 8888
#define MAX_FD 65536
#define MAX_EVENT_NUMBER 10000

extern int addfd(int epollfd, int fd, bool one_shot);
extern int removefd(int epollfd, int fd);

void addsig(int sig, void(handler)(int), bool restart = true) {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if(restart) {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, nullptr) != -1);
}

void show_error(int connfd, const char* info) {
    printf("%s", info);
    send(connfd, info, strlen(info), 0);
    close(connfd);
}


int main(int argc, char* argv[]) {
    int port = DEFUALT_PORT;
    if(argc> 1) {
        port = atoi(argv[1]);
    }

    // 忽略SIGPIPE信号
    addsig(SIGPIPE, SIG_IGN);

    // 创建线程池
    threadpool<http_conn>* pool = nullptr;
    try {
        pool = new threadpool<http_conn>;
    }
    catch(...) {
        return 1;
    }

    // 预先为每个可能的客户连接分配一个http_conn对象
    http_conn* users = new http_conn[MAX_FD];
    assert(users);
    int user_count = 0;

    // 创建socket
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd>= 0);

    // 设置linger
    struct linger tmp = { 1, 0 };
    setsockopt(listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));

    int ret = 0;
    struct sockaddr_in address; // 创建socket地址
    bzero(&address, sizeof(address)); // 清空地址
    address.sin_family = AF_INET; //选择协议族，IPv4
    address.sin_addr.s_addr = htonl(INADDR_ANY); // 监听通配IP地址
    address.sin_port = htons(port); // 绑定端口号

    ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address)); // 命名socket
    assert(ret>= 0);

    ret = listen(listenfd, 5); // 监听socket
    assert(ret>= 0);

    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5); // 创建epoll事件监听表，epolfd文件描述符用于唯一标识内核事件表
    assert(epollfd != -1);

    /** 注意，监听 socket listenfd 上是不能注册 EPOLLONESHOT 事件的，否则应用程序只能处理
    一个客户连接！ 因为后续的客户连接请求将不再触发 listenfd 上的 EPPLLIN 事件 */
    addfd(epollfd, listenfd, false); // 注册listenfd上的事件到epollfd指示的内核事件表

    http_conn::m_epollfd = epollfd;

    while(true) {
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1); // 等待就绪事件
        if ((number <0) && (errno != EINTR)) {
            printf("epoll failure\n");
            break;
        }

        // 处理就绪的文件描述符
        for (int i = 0; i <number; i++) {
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd) {
                struct sockaddr_in client_addr;
                socklen_t client_addrlength = sizeof(client_addr);
                int connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addrlength);
                if (connfd <0) {
                    printf("errno is: %d\n", errno);
                    continue;
                }
                if(http_conn::m_user_count>= MAX_FD) {
                    show_error(connfd, "Internal server busy");
                    continue;
                }

                // 初始化客户连接
                users[connfd].init(connfd, client_addr);

                // 连接成功，打印客户端的IP地址和端口号
                printf("client connected with ip: %s port: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            }
            else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                // 如果有异常，直接关闭客户连接
                users[sockfd].close_conn();
            }
            else if(events[i].events & EPOLLIN) {
                // 根据读的结果，决定是将任务添加到线程池，还是关闭连接
                if(users[sockfd].read()) {
                    pool->append(users + sockfd);
                }
                else {
                    users[sockfd].close_conn();
                }
            }
            else if(events[i].events & EPOLLOUT) {
                // 根据写的结果，决定是否关闭连接
                if(!users[sockfd].write()) {
                    users[sockfd].close_conn();
                }
            }
            else {}
        }
    }

    close(epollfd);
    close(listenfd);
    delete [] users;
    delete pool;
    return 0;
}
