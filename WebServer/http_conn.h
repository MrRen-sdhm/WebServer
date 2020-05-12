#pragma once

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>

/**
 * 主状态机：用于分析HTTP报文的请求行、头部字段、内容
 * 从状态机：用于解析一行HTTP报文
 */

class http_conn
{
public:
    static const int FILENAME_LEN = 200; // 文件名最大长度
    static const int READ_BUFFER_SIZE = 2048; // 读缓冲区大小
    static const int WRITE_BUFFER_SIZE = 1024; // 写缓冲区大小
    enum METHOD { GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATCH }; // HTTP请求的方法，目前仅支持GET
    enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER, CHECK_STATE_CONTENT }; // 主状态机的可能状态
    enum HTTP_CODE { NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, // 处理HTTP请求的可能结果
                     FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };
    enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN }; // 行的读取状态，即从状态机状态

public:
    http_conn(){}
    ~http_conn(){}

public:
    void init(int sockfd, const sockaddr_in& addr); // 初始化新接受的连接
    void close_conn(bool real_close = true); // 关闭连接
    void process(); // 处理客户请求
    bool read(); // 非阻塞读操作
    bool write(); // 非阻塞写操作

private:
    void init(); // 初始化连接
    HTTP_CODE process_read(); // 解析HTTP请求
    bool process_write(HTTP_CODE ret); // 填充HTTP应答

    /* 以下函数被process_read函数调用，用于分析HTTP请求 */
    HTTP_CODE parse_request_line(char* text);
    HTTP_CODE parse_headers(char* text);
    HTTP_CODE parse_content(char* text);
    HTTP_CODE do_request();
    char* get_line() { return m_read_buf + m_start_line; }
    LINE_STATUS parse_line(); // 从状态机

    /* 以下函数被process_read函数调用，用于填充HTTP应答 */
    void unmap();
    bool add_response(const char* format, ...);
    bool add_content(const char* content);
    bool add_status_line(int status, const char* title);
    bool add_headers(int content_length);
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

public:
    static int m_epollfd; // 所有socket上的事件都被注册到同一个epoll的内核事件表中
    static int m_user_count; // 统计用户数量

private:
    int m_sockfd; // 该HTTP连接的socket
    sockaddr_in m_address; // 对方的socket地址

    char m_read_buf[READ_BUFFER_SIZE]; // 读缓冲区
    int m_read_idx; // 标识读缓冲中已经读入的客户数据的最后一个字节的下一个未知位置
    int m_checked_idx; // 当前正在分析的字符在读缓冲区中的位置
    int m_start_line; // 当前正在解析的行的起始位置
    char m_write_buf[WRITE_BUFFER_SIZE]; // 写缓冲区
    int m_write_idx; // 写缓冲区中待发送的字节数

    CHECK_STATE m_check_state; // 主状态机所处的状态
    METHOD m_method; // 请求方法

    char m_real_file[FILENAME_LEN]; // 客户请求的目标文件的完整路径，等于doc_root + m_url，doc_root为网站根目录
    char* m_url; // 客户请求的目标文件的文件名
    char* m_version; // HTTP协议版本号，目前仅支持HTTP/1.1
    char* m_host; // 主机名
    int m_content_length; // 请求的消息体的长度
    bool m_linger; // HTTP请求是否要保持连接

    char* m_file_address; // 客户请求的目标文件被mmap到的内存的起始位置
    struct stat m_file_stat; // 目标文件的状态，可通过它判断文件是否存在、是否为目录、是否可读，并获取文件大小等信息

    // 通过writev来执行写操作，需定义两个成员
    struct iovec m_iv[2];
    int m_iv_count; // 被写内存块的数量
};