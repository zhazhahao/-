#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8888
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    int len;

    // 创建socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // 绑定地址和端口
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(1);
    }

    // 监听端口
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        exit(1);
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1) {
        // 接受客户端连接
        len = sizeof(client_addr);
        if ((client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &len)) == -1) {
            perror("accept");
            continue;
        }

        printf("Client %s:%d connected.\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        while (1) {
            // 接收客户端数据
            memset(buffer, 0, sizeof(buffer));
            if (recv(client_fd, buffer, sizeof(buffer), 0) == -1) {
                perror("recv");
                break;
            }

            if (strlen(buffer) == 0) {
                printf("Client %s:%d disconnected.\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                break;
            }

            printf("Received from client %s:%d: %s", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);

            // 发送数据给客户端
            if (send(client_fd, buffer, strlen(buffer), 0) == -1) {
                perror("send");
                break;
            }
        }

        // 关闭客户端连接
        close(client_fd);
    }

    // 关闭服务端socket
    close(server_fd);

    return 0;
}
