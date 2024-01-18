#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#define BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFFER_SIZE];

    if (argc < 3) {
        fprintf(stderr, "Usage: %s hostname port\n", argv[0]);
        exit(1);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    pid_t pid = fork();
    if (pid < 0)
        error("ERROR on fork");

    if (pid == 0) {
        // Child process
        while (1) {
            printf("Please enter the message: ");
            bzero(buffer, BUFFER_SIZE);
            fgets(buffer, BUFFER_SIZE - 1, stdin);

            n = write(sockfd, buffer, strlen(buffer));
            if (n < 0)
                error("ERROR writing to socket");

            bzero(buffer, BUFFER_SIZE);
            n = read(sockfd, buffer, BUFFER_SIZE - 1);
            if (n < 0)
                error("ERROR reading from socket");

            printf("Server response: %s\n", buffer);
        }
    } else {
        // Parent process
        wait(NULL);
        close(sockfd);
    }

    return 0;
}
