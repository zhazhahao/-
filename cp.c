#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>

// 定义错误打印函数，打印错误信息并退出程序
#define printError(s)  perror(s), exit(1);

extern int errno;

// 返回两个整数中的较大值
int max(int a, int b) {
    return a >= b ? a : b;
}

// 检查给定路径是否为目录
bool isDirectory(char* path) {
    struct stat ss;
    stat(path, &ss);
    return S_ISDIR(ss.st_mode) ? true : false;
}

char src[500], tar[500];  // 源文件路径，目标文件路径
char *buf;  // 读取源文件内容
char op[30];  // 目标文件存在时，覆盖还是追加
int src_path_len, tar_path_len, buf_len;  // 源文件路径/目标文件路径/读取的内容长度
int src_length;  // 源文件内容大小

// 复制文件到文件
int file_to_file(char *src, char *tar) {
    int fd_src, fd_tar;  // 源文件描述符，目标文件描述符

    // 打开源文件
    if ((fd_src = open(src, O_RDWR)) == -1)
        printError(src);  // 源文件不存在或打开失败

    // 获取源文件大小
    if ((src_length = lseek(fd_src, 0, SEEK_END)) == -1)
        printError(src);  // lseek读取源文件大小失败

    lseek(fd_src, 0, SEEK_SET);  // 源文件定位到文件首，以便下面进行read

    // 动态分配内存，存储源文件内容
    buf = (char *)malloc(sizeof(char) * src_length * 10);

    // 读取源文件内容到缓冲区
    if (read(fd_src, buf, max(0, src_length - 1)) == -1)
        printError(src);  // 源文件读取内容失败

    if (isDirectory(tar)) {  // 目标文件是目录，处理出新的目标路径(默认复制后的文件和源文件同名)
        src_path_len = strlen(src);
        tar_path_len = strlen(tar);
        int pos = 0;
        int i = 0;
        for (i = 0; i < src_path_len; i++)
            if (src[i] == '/')
                pos = i;
        if (pos == 0)
            tar[tar_path_len++] = '/';
        for (i = pos; i < src_path_len; i++)
            tar[tar_path_len++] = src[i];
    }
    printf("source path:%s\ntarget path:%s\n", src, tar);

    // 如果目标文件不存在，则创建该文件
    if (open(tar, O_RDWR) == -1) {
        if ((fd_tar = open(tar, O_RDWR | O_CREAT, 0666)) == -1)
            printError(tar);  // 创建目标文件失败
    } else {
        printf("Object file already exists, which one do you want to operate, overwrite or append?\n");
        printf("Please input overwrite(o) or append(a)?\n");
        scanf("%s", op);
        while (1) {
            if (op[0] == 'o') {  // 覆盖
                if ((fd_tar = open(tar, O_RDWR | O_TRUNC)) == -1)
                    printError(tar);
                break;
            } else if (op[0] == 'a') {  // 追加
                if ((fd_tar = open(tar, O_RDWR | O_APPEND)) == -1)
                    printError(tar);
                break;
            } else
                printf("Please input overwrite(o) or append(a)?\n");
        }
    }

    // 将源文件内容写入目标文件
    if (write(fd_tar, buf, max(0, src_length - 1)) == -1)
        printError(tar);  // 写入文件失败

    printf("All done!\n");

    close(fd_src);
    close(fd_tar);
    free(buf);
}

// 复制目录到目录
int dir_to_dir(char *src, char *tar) {
    DIR *pdir1 = opendir(src);
    struct dirent *pdirent;
    while ((pdirent = readdir(pdir1))) {
        if (!strcmp(pdirent->d_name, ".") || !strcmp(pdirent->d_name, "..")) {
            //.和..文件不操作
            continue;
        }
        char old[512] = {0};
        char new[512] = {0};
        sprintf(old, "%s%s%s", src, "/", pdirent->d_name);
        sprintf(new, "%s%s%s", tar, "/", pdirent->d_name);
        if (pdirent->d_type == DT_DIR) {  // 是目录，继续递归
            int ret = mkdir(new, 0777);
            dir_to_dir(old, new);
        } else {  // 是文件，复制
            file_to_file(old, new);
        }
    }
    closedir(pdir1);
    return 0;
}

int main(int argc, char *argv[]) {
    // 源文件路径和目标文件路径缺失
    if (argc == 1)
        printf("ERROR:If you want to implement the cp command function, please input the source and target path!\n");
    // 目标文件路径缺失
    else if (argc == 2)
        printf("ERROR:Please input the target path!\n");
    else {
        strcat(src, argv[1]);
        strcat(tar, argv[2]);

        if (isDirectory(src)) {  // 源路径是目录
            dir_to_dir(src, tar);
        } else {  // 源路径是文件
            file_to_file(src, tar);
        }
    }
    return 0;
}
