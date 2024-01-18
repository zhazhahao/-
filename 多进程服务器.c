
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <assert.h>

#define BACKLOG 1024
#define bprintf(fp, format, ...) \
    if (fp == NULL) { \
        printf(format, ##__VA_ARGS__); \
    } else { \
        printf(format, ##__VA_ARGS__); \
        fprintf(fp, format, ##__VA_ARGS__); \
        fflush(fp); \
    }

int sig_type = 0, sig_to_exit = 0;
FILE* fp_res = NULL;

void sig_int(int signo) {
    sig_type = signo;
    pid_t pid = getpid();
    bprintf(fp_res, "[srv](%d) SIGINT ....", pid);
    sig_to_exit = 1;
}

void sig_pipe(int signo) {
    // 处理 SIGPIPE 信号
}

void sig_chld(int signo) {
    // 处理 SIGCHLD 信号
    pid_t pid;
    int stat;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        // 子进程终止
        bprintf(fp_res, "[srv](%d) 子进程(%d) 终止\n", getpid(), pid);
    }
}

int install_sig_handlers() {
    int res = -1;
    struct sigaction sigact_pipe, old_sigact_pipe;
    sigact_pipe.sa_handler = sig_pipe;
    sigact_pipe.sa_flags = 0;
    sigact_pipe.sa_flags |= SA_RESTART;
    sigemptyset(&sigact_pipe.sa_mask);
    res = sigaction(SIGPIPE, &sigact_pipe, &old_sigact_pipe);
    if (res)
        return -1;

    struct sigaction sigact_chld, old_sigact_chld;
    sigact_chld.sa_handler = sig_chld;
    sigact_chld.sa_flags = 0;
    sigact_chld.sa_flags |= SA_RESTART;
    sigemptyset(&sigact_chld.sa_mask);
    res = sigaction(SIGCHLD, &sigact_chld, &old_sigact_chld);
    if (res)
        return -2;

    struct sigaction sigact_int, old_sigact_int;
    sigact_int.sa_handler = sig_int;
    sigact_int.sa_flags = 0;
    sigemptyset(&sigact_int.sa_mask);
    res = sigaction(SIGINT, &sigact_int, &old_sigact_int);
    if (res)
        return -3;

    return 0;
}

int echo_rep(int sockfd) {
    // 处理客户端请求
    char buffer[4096];
    ssize_t bytes_received, bytes_sent;

    while (1) {
        // 从客户端接收数据
        bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            break;
        }

        // 将接收到的数据发送回客户端
        bytes_sent = send(sockfd, buffer, bytes_received, 0);
        if (bytes_sent <= 0) {
            break;
        }
    }

    return 0;
}

int main(int argc, char* argv[]) {
    // 检查命令行参数
    if (argc != 3) {
        printf("Usage: %s <ip_address> <port>\n", argv[0]);
        return -1;
    }

    // 获取进程 ID
    pid_t pid = getpid();
    bprintf(fp_res, "[srv](%d) 服务器[%s:%s]正在初始化...\n", pid, argv[1], argv[2]);

    // 创建服务器套接字
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    // 设置服务器地址
    struct sockaddr_in srv_addr;
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &(srv_addr.sin_addr));

    // 将套接字绑定到服务器地址
    int res = bind(listenfd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    assert(res != -1);

    // 开始监听客户端连接
    res = listen(listenfd, BACKLOG);
    assert(res != -1);

    bprintf(fp_res, "[srv](%d) 服务器[%s:%s]正在监听...\n", pid, argv[1], argv[2]);

    // 安装信号处理函数
    res = install_sig_handlers();
    assert(res == 0);

    // 为每个客户端连接创建一个子进程
    while (!sig_to_exit) {
        struct sockaddr_in cli_addr;
        socklen_t cli_addr_len = sizeof(cli_addr);
        int connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &cli_addr_len);
        if (connfd < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                bprintf(fp_res, "[srv](%d) 接受连接失败\n", pid);
                break;
            }
        }

        // Fork一个子进程
        pid_t child_pid = fork();
        if (child_pid == 0) {
            // 子进程
            close(listenfd);
            echo_rep(connfd);
            bprintf(fp_res, "[srv](%d) 子进程终止。\n", getpid());
            exit(0);
        } else if (child_pid > 0) {
            // 父进程
            close(connfd);
        } else {
            // Fork错误
            bprintf(fp_res, "[srv](%d) Fork错误\n", pid);
            break;
        }
    }

    // 清理并退出
    close(listenfd);
    if (fp_res != NULL) {
        fclose(fp_res);
    }
    return 0;
}
