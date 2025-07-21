#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

int flag = 0;

void inter_handler(int signo) {
    if(signo == SIGINT || signo == SIGQUIT || signo == SIGALRM)
        flag = signo;
}

void child1_handler(int signo) {
    // child1 process to be killed
    printf("\n 16 stop test\n");
    printf("\nChild process1 is killed by parent!!\n");
    exit(0);
}

void child2_handler(int signo) {
    // child process2 to be killed
    printf("\n 17 stop test\n");
    printf("\nChild process2 is killed by parent!!\n");
    exit(0);
}

void waiting() {
    while(flag == 0) ;  // spin and wait until flag is changed
}

int main() {
    signal(SIGINT, inter_handler);
    signal(SIGQUIT, inter_handler);
    signal(SIGALRM, inter_handler);
    alarm(5);
    pid_t pid1 = -1, pid2 = -1;
    while(pid1 == -1) pid1 = fork();
    if(pid1 > 0) {
        while(pid2 == -1) pid2 = fork();
        if(pid2 > 0) {
            // parent process
            waiting();
            printf("\n %d stop test\n", flag);

            // kill child process
            kill(pid1, 16);
            kill(pid2, 17);

            // wait for stop of child process
            wait(NULL);
            wait(NULL);

            printf("\nParent process is killed!!\n");
        }
        else {
            // child process2
            signal(17, child2_handler);
            while(1){
                ;
            }
            return 0;
        }
    }
    else {
        // child process1
        signal(16, child1_handler);
        while(1) {
            ;
        }
        return 0;
    }
}