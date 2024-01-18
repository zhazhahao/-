

#include <stdio.h>
#include <stdlib.h>     //exit()函数，atoi()函数
#include <unistd.h>     //提供对 POSIX 操作系统 API 的访问功能的头文件
#include <sys/types.h>  //Linux系统的基本系统数据类型的头文件,含有size_t,time_t,pid_t等类型
#include <sys/socket.h> //套接字基本函数
#include <netinet/in.h> //IP地址和端口相关定义，比如struct sockaddr_in等
#include <arpa/inet.h>  //inet_pton()等函数
#include <string.h>     //bzero()函数

#define MAXDATASIZE 120 //定义一个常量

void clear_input_buffer(void) //用于读取标准输入缓冲区的函数
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main(int argc, char *argv[]) //程序的入口函数，argc保存命令行输入的参数个数，argv保存命令行输入的参数值
{

    int clientfd, numbytes; //定义客户端套接字和客户端接收到的字节数变量
    char buf_in[MAXDATASIZE + 2]; //定义缓冲区变量，用于存放从标准输入读取的信息
    char buf_out[MAXDATASIZE + 17]; //定义缓冲区变量，用于存放从服务器接收到的信息
    struct sockaddr_in server_addr; //定义存放服务器端地址信息的结构体变量，用于connect()函数使用  

    if (argc != 3)
    { // 如果命令行用法不对，则提醒并退出
        printf("usage: %s  <server IP address>  <server port>\n", argv[0]);
        exit(0);
    }

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) //创建客户端套接字，失败则输出错误提示并退出程序
    {
        perror("Create socket failed.");
        exit(1);
    }

    bzero(&server_addr, sizeof(server_addr)); //将server_addr结构体变量的大小都设置为0，清空结构体变量
    server_addr.sin_family = AF_INET; //设置协议族为AF_INET

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) == 0) //将argv[1]字符串转换为IP地址，并存放到server_addr.sin_addr中
    {
        perror("Server IP Address Error:");
        exit(1);
    }

    server_addr.sin_port = htons(atoi(argv[2])); //将argv[2]字符串转换为端口，并用htons函数将端口号转换为网络字节序，存放到server_addr.sin_port中

    if (connect(clientfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) //连接服务器，失败则输出错误提示并退出程序
    {
        perror("connect failed.");
        exit(1);
    }
    else
        printf("[cli] server[%s:%d] is connected!\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port)); //连接成功，输出提示信息

    while (1)
    {
        fgets(buf_in, 122, stdin); //从标准输入读取信息，并存放到buf_in中
        ssize_t s = strlen(buf_in);
        buf_in[s] = '\0'; //确保buf_in以空字符结尾

        printf("[ECH_RQT]%s", buf_in); //输出从标准输入读取的信息

        char checkchar[6] = {'E', 'X', 'I', 'T', '\n', '\0'}; //定义一个char数组，存放字符串"EXIT\n"

        if (!strncmp(buf_in, checkchar, 6)) //如果buf_in等于"EXIT\n"，则退出
        {
            break;
        }

        if (s > 0)
        {
            write(clientfd, buf_in, s + 1); //将buf_in发送给服务器
            ssize_t _s = read(clientfd, buf_out, 8 + strlen(buf_in)); //从服务器读取回复信息

            if (_s > 0)
            {
                printf("[ECH_REP]%s", buf_out); //输出从服务器读取的信息
            }
        }
    }

    close(clientfd); //关闭客户端套接字
    printf("[cli] connfd is closed!\n[cli] client is to return!\n"); //输出提示信息
    fflush(stdout);
    return 0; //返回0，表示程序正常运行结束
}