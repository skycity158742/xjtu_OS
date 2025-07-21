#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int pid1, pid2;

int main() {
    int fd[2];
    char InPipe[5000]; // read buffer
    char* inpipe;
    inpipe = InPipe;
    char c1 = '1', c2 = '2';
    int size = sizeof(char);
    pipe(fd); // create pipe
    while((pid1 = fork()) == -1){
        ; // fork failed, spin
    }
    if(pid1 == 0) {
        // child process1
        int i;
        lockf(fd[1], 1, 0);// lock pipe

        // write '1' into pipe for 2000 times
        for(i = 0; i < 2000; i++) {
            write(fd[1], &c1, size);
        }

        sleep(5); // wait for reading
        lockf(fd[1], 0, 0); // unlock pipe

        exit(0);
    }
    else {
        while((pid2 = fork()) == -1) {
            ; // fork failed, spin
        }
        if(pid2 == 0) {
            // child process2
            int j;
            lockf(fd[1], 1, 0); // lock pipe
            
            // write '2' into pipe for 2000 times
            for(j = 0; j < 2000; j++) {
                write(fd[1], &c2, size);
            }
            
            sleep(5); // wait for reading
            lockf(fd[1], 0, 0);//unlock pipe
            exit(0);
        }
        else {
            wait(NULL);// wait for stop of process1
            wait(NULL);// wait for stop of process2
            // read 4000 char from pipe
            close(fd[1]);
            read(fd[0], inpipe, 4000);
            *(inpipe + 4000) = '\0';// add end status
            printf("%s\n", InPipe);
            exit(0); // end of parent process
        }
    }
}