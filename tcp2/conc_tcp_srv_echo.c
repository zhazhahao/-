#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#define MAX_BACKLOG 5
#define MAX_BUFFER_SIZE 220

volatile sig_atomic_t sigint_flag = 0;

void handle_sigint(int sig) {
    printf("[srv] SIGINT is coming!\n");
    sigint_flag = 1;
}

void handle_sigchld(int sig) {
    pid_t pid_chld;
    int stat;
    while ((pid_chld = waitpid(-1, &stat, WNOHANG)) > 0) {
        printf("[srv](%d)[chd](%d) Child has terminated!\n", getpid(), pid_chld);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <server_ip> <server_port> <verification_code>\n", argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int verification_code = atoi(argv[3]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1){
        perror("Failed to create socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return 1;
    }

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        return 1;
    }

    if (listen(sockfd, MAX_BACKLOG) == -1) {
        perror("Listen failed");
        return 1;
    }

    struct sigaction sa_int;
    sa_int.sa_flags = 0;
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sigaction(SIGINT, &sa_int, NULL);

    struct sigaction sa_chld;
    sa_chld.sa_flags = SA_RESTART;
    sa_chld.sa_handler = handle_sigchld;
    sigemptyset(&sa_chld.sa_mask);
    sigaction(SIGCHLD, &sa_chld, NULL);

    printf("[srv](%d)[srv_sa](%s:%d)[vcd](%d) Server has initialized!\n",
           getpid(), server_ip, server_port, verification_code);

    while (!sigint_flag) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        int connfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (connfd == -1) {
            if (errno == EINTR)
                continue;
            else {
                perror("Accept failed");
                break;
            }
        }

        printf("[srv](%d)[cli_sa](%s:%d) Client is accepted!\n",getpid(), inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        pid_t pid = fork();
        if (pid == -1) {
            perror("Fork failed");
            break;
        } else if (pid == 0) {
            // Child process
            printf("[chd](%d)[ppid](%d) Child process is created!\n", getpid(), getppid());
            close(sockfd);
            char buffer[MAX_BUFFER_SIZE]={0};
            ssize_t bytes_read;

            while ((bytes_read = read(connfd, buffer, sizeof(buffer) - 1)) > 0) {
                
                printf("[chd](%d)[cid](%d)[ECH_RQT] %s\0", getpid(), verification_code, buffer);

                if (write(connfd, buffer, bytes_read) != bytes_read) {
                    perror("Write failed");
                    break;
                }
            }

            close(connfd);
            return 0;
        } else {
            // Parent process
            close(connfd);
        }
    }

    printf("[srv](%d) Server is to return!\n", getpid());
    close(sockfd);

    return 0;
}
