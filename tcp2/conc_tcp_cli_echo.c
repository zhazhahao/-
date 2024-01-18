#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 200

void cli_biz(int sockfd, int client_id) {
    char buffer[MAX_BUFFER_SIZE];

    while (1) {
       
        fflush(stdout);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            break;

        if (strcmp(buffer, "EXIT\n") == 0)
            break;

        size_t message_len = strlen(buffer);
        if (write(sockfd, buffer, message_len) != message_len) {
            perror("Write failed");
            break;
        }

        printf("[cli](%d)[cid](%d)[ECH_RQT] %s", getpid(), client_id, buffer);
         // Receive and print the response
        char response[MAX_BUFFER_SIZE];
        ssize_t bytes_read = read(sockfd, response, sizeof(response));
        if (bytes_read > 0) {
            response[bytes_read] = '\0';
            printf("[cli](%d)[vcd](%d)[ECH_REP] %s", getpid(), client_id, response);
        } else {
            perror("Read failed");
            break;
        }
    }

    printf("[cli](%d) Client is to return!\n", getpid());
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <server_ip> <server_port> <client_id>\n", argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int client_id = atoi(argv[3]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
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

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    printf("[cli](%d)[srv_sa](%s:%d) Server is connected!\n", getpid(), server_ip, server_port);

    cli_biz(sockfd, client_id);

    close(sockfd);

    return 0;
}
