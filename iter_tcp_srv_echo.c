// 包含需要使用的头文件
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// 定义监听队列长度
#define BACKLOG 5

// 定义最大读取长度
#define MAX_SIZE 150

// 定义全局变量sigint_flag，用于标记是否收到中断信号SIGINT
int sigint_flag = 0;

// 信号处理函数，用于处理SIGINT信号
void handle_sigint(int sig)
{
    printf("[srv] SIGINT is coming!\n");
    sigint_flag = 1; // 更改sigint_flag的值，标记收到SIGINT信号
}

int main(int argc, char *argv[])
{
    // 定义SIGINT信号处理结构体sa，并设置相应的属性
    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);

    // 将SIGINT信号与上述处理结构体sa关联起来
    sigaction(SIGINT, &sa, NULL);

    // 定义监听套接字listenfd和连接套接字connectfd
    int listenfd, connectfd, strNumber, new_sock;

    // 定义服务器地址信息server和客户端地址信息client
    struct sockaddr_in server, client;

    // 定义待accept客户端地址信息的长度
    int sin_size;

    // 校验命令行参数是否正确
    if (argc != 4)
    {
        printf("usage: %s <IP address> <port> <veri_code>\n", argv[0]);
        exit(0);
    }

    // 创建一个监听套接字listenfd
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Create socket failed.");
        exit(-1);
    }

    // 设置监听套接字的地址和端口为可立即重用
    int opt = SO_REUSEADDR;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 初始化服务器地址信息server
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    server.sin_addr.s_addr = inet_addr(argv[1]);

    // 将监听套接字与server地址信息绑定
    if (bind(listenfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("Bind error.");
        exit(-1);
    }

    // 设置监听套接字，并设置队列长度为BACKLOG
    if (listen(listenfd, BACKLOG) == -1)
    {
        perror("listen error.");
        exit(-1);
    }
    printf("[srv] server[%s:%s][%s] is initializing!\n", argv[1], argv[2], argv[3]);
    printf("[srv] server has initialized!\n");

    // 循环监听客户端请求，直到收到SIGINT信号
    sin_size = sizeof(struct sockaddr_in);
    while (!sigint_flag)
    {
        // 通过accept函数接收客户端请求，得到连接套接字connectfd
        new_sock = accept(listenfd, (struct sockaddr *)&client, &sin_size);

        // 处理accept函数返回的出错情况
        if (new_sock < 0)
        {
            if (errno == EINTR) // 如果调用被信号中断了，则继续循环
            {
                continue;
            }
            else // 否则输出错误信息，并结束循环
            {
                perror("accept");
                break;
            }
        }

        // 输出客户端连接成功的信息
        printf("[srv] client[%s:%d] is accepted!\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        // 通过读写套接字通信
        char vri_code[8];
        memset(vri_code, '\0', 8);
        sprintf(vri_code, "(%s)", argv[3]);
        char buf[MAX_SIZE];

        // 先read后write
        while (1)
        {
            ssize_t s = read(new_sock, buf, MAX_SIZE);
            if (s > 0)
            {
                printf("[ECH_RQT]%s", buf);
                char send_buf[strlen(vri_code) + s];
                memset(send_buf, '\0', strlen(vri_code) + s + 1);
                strcpy(send_buf, vri_code);
                strcat(send_buf, buf);
                int endpoint = strlen(send_buf);
                send_buf[endpoint] = '\0';
                write(new_sock, send_buf, strlen(send_buf) + 1);
            }
            else if (s == 0) // read函数返回值为0，表示客户端关闭了连接
            {
                printf("[srv] client[%s:%d] is closed!\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

                // 关闭连接套接字
                close(new_sock);
                break;
            }
            else // read函数返回值小于0，出现错误，退出循环
            {
                perror("read");
                break;
            }
        }
    }

    // 关闭监听套接字，并输出关闭监听套接字的信息
    close(listenfd);
    printf("[srv] listenfd is closed!\n[srv] server is to return!\n");
    return 0;
}