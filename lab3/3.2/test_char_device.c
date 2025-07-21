#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h> // read, write, close
#include <stdlib.h> // exit

int main()
{
    int fd;
    char num[101], temp;

    printf("1>write\n2>read\n");
    temp = getchar();
    getchar(); // 清除换行符

    if (temp == '2') { // 读
        fd = open("/dev/ch_device", O_RDWR, S_IRUSR | S_IWUSR); // 修改设备路径为 /dev/ch_device
        if (fd == -1) {
            perror("Device open failure");
            exit(EXIT_FAILURE);
        }

        printf("Reading from /dev/ch_device:\n");
        while (1) {
            memset(num, '\0', sizeof(num));
            int ret = read(fd, num, 100);
            
            if (ret < 0) {
                perror("Read failed");
                close(fd);
                exit(EXIT_FAILURE);
            } else if (ret == 0) {
                printf("No data to read\n");
                int waitingflag = 0;
                printf("Waiting for data or not?\nEnter 0 to wait, and other numbers to quit waiting.\n");
                scanf("%d", &waitingflag);
                if(!waitingflag) {
                    close(fd);
                    return 0;
                }
                continue; // 等待数据
            }
            num[ret] = '\0'; // 确保字符串以 '\0' 结尾

            printf("Read complete, as follows:\n%s\n", num);
            printf("Continue reading or not?\nEnter 1 to quit reading, and other numbers to continue.\n");
            int quitflag = 0;
            scanf("%d", &quitflag);
            if(quitflag == 1) break;
            else continue;
            // if (strcmp(num, "exit") == 0) {
            //     printf("Exiting read loop\n");
            //     break;
            // }
        }
        close(fd);

    } else if (temp == '1') { // 写
        fd = open("/dev/ch_device", O_RDWR, S_IRUSR | S_IWUSR); // 修改设备路径为 /dev/ch_device
        if (fd == -1) {
            perror("Device open failure");
            exit(EXIT_FAILURE);
        }

        printf("Writing to /dev/ch_device:\n");
        while (1) {
            printf("Please input message (type 'exit' to quit):\n");
            if (fgets(num, sizeof(num), stdin) == NULL) {
                perror("Input error");
                break;
            }
            num[strcspn(num, "\n")] = '\0'; // 去掉换行符

            int ret = write(fd, num, strlen(num));
            if (ret < 0) {
                perror("Write failed");
                close(fd);
                exit(EXIT_FAILURE);
            }

            if (strcmp(num, "exit") == 0) {
                printf("Exiting write loop\n");
                break;
            }
        }
        close(fd);

    } else {
        printf("Invalid option\n");
    }

    return 0;
}
