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

#define BACKLOG 5
#define MAX_SIZE 150

volatile int sigint_flag = 0;

void handle_sigint(int sig)
{
    printf("[srv] SIGINT is coming!\n");
    sigint_flag = 1;
}

void accept_client(int listenfd, struct sockaddr_in* client) {
    int sin_size = sizeof(struct sockaddr_in);
    int new_sock = accept(listenfd, (struct sockaddr *)client, &sin_size);

    if (new_sock < 0)
    {
        if (errno == EINTR)
        {
            accept_client(listenfd, client);
        }
        else
        {
            perror("accept");
            return;
        }
    }

    printf("[srv] client[%s:%d] is accepted!\n", inet_ntoa(client->sin_addr), ntohs(client->sin_port));

    char vri_code[8];
    memset(vri_code, '\0', 8);
    sprintf(vri_code, "(%s)", argv[3]);
    char buf[MAX_SIZE];

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
        else if (s == 0)
        {
            printf("[srv] client[%s:%d] is closed!\n", inet_ntoa(client->sin_addr), ntohs(client->sin_port));
            close(new_sock);
            break;
        }
        else
        {
            perror("read");
            break;
        }
    }
}

void start_server(char* ip_address, char* port, char* veri_code) {
    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGINT, &sa, NULL);

    int listenfd, connectfd;

    struct sockaddr_in server, client;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Create socket failed.");
        exit(-1);
    }

    int opt = SO_REUSEADDR;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port));
    server.sin_addr.s_addr = inet_addr(ip_address);

    if (bind(listenfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("Bind error.");
        exit(-1);
    }

    if (listen(listenfd, BACKLOG) == -1)
    {
        perror("listen error.");
        exit(-1);
    }
    printf("[srv] server[%s:%s][%s] isinitializing!\n", ip_address, port, veri_code);
    printf("[srv] server has initialized!\n");

    while (!sigint_flag)
    {
        accept_client(listenfd, &client);
    }

    close(listenfd);
    printf("[srv] listenfd is closed!\n[srv] server is to return!\n");
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("usage: %s <IP address> <port> <veri_code>\n", argv[0]);
        exit(0);
    }

    start_server(argv[1], argv[2], argv[3]);

    return 0;
}
