#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

int main()
{
    int fd[2];
    if (pipe(fd) == -1)
    {
    
        return -1;
    }

    pid_t pid1 = fork();
    if (pid1 < 0)
    {
        perror("Failed to create the pipe");
        return -1;
    }
    else if (pid1 == 0)
    { // 子进程1
        char *msg1 = "Child process 1 is sending a message!\n";
        close(fd[0]);
        write(fd[1], msg1, strlen(msg1)); 

        return 0;
    }
    else
    {   waitpid(pid1, NULL, 0); 
        pid_t pid2 = fork();
        if (pid2 < 0)
        {
            perror("Failed to create the pipe");
            return -1;
        }
        else if (pid2 == 0)
        { // 子进程2
            char *msg2 = "Child process 2 is sending a message!\n";
            close(fd[0]);
            write(fd[1], msg2, strlen(msg2)); 

            return 0;
        }
        else
        {                          
            
            waitpid(pid2, NULL, 0); 

            char buf[100] = {0};
            close(fd[1]);          
            read(fd[0], buf, 100); 
            printf("%s", buf);

            return 0;
        }
    }
}
