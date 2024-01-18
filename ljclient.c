#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define MAXDATASIZE 120

void clear_input_buffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void connect_to_server(char* server_ip, char* server_port)
{
    int clientfd, numbytes;
    char buf_in[MAXDATASIZE + 2];
    char buf_out[MAXDATASIZE + 17];
    struct sockaddr_in server_addr;

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Create socket failed.");
        exit(1);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) == 0)
    {
        perror("Server IP Address Error:");
        exit(1);
    }

    server_addr.sin_port = htons(atoi(server_port));

    if (connect(clientfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("connect failed.");
        exit(1);
    }
    else
        printf("[cli] server[%s:%d] is connected!\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

    while (1)
    {
        fgets(buf_in, 122, stdin);
        ssize_t s = strlen(buf_in);
        buf_in[s] = '\0';

        printf("[ECH_RQT]%s", buf_in);

        char checkchar[6] = {'E', 'X', 'I', 'T', '\n', '\0'};

        if (!strncmp(buf_in, checkchar, 6))
        {
            break;
        }

        if (s > 0)
        {
            write(clientfd, buf_in, s + 1);
            ssize_t _s = read(clientfd, buf_out, 8 + strlen(buf_in));

            if (_s > 0)
            {
                printf("[ECH_REP]%s", buf_out);
            }
        }
    }

    close(clientfd);
    printf("[cli] connfd is closed!\n[cli] client is to return!\n");
    fflush(stdout);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("usage: %s  <server IP address>  <server port>\n", argv[0]);
        exit(0);
    }

    connect_to_server(argv[1], argv[2]);

    return 0;
}
