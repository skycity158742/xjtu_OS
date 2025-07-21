#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>


int main()
{    
pid_t pid, pid1;
    
    /* fork a child process */
    pid = fork();

    if(pid < 0){ /* error occured */
        fprintf(stderr, "Fork Failed!");
        return 1;
    }
    else if(pid == 0){ /* child process */
        execlp("./system_call", "system_call", NULL);
        /* If execlp() failed, the following child process will NOT be covered and will be executed to tell an error */
        perror("exec");
        exit(1);
    }
    else{/* parent process */
        pid1 = getpid();
        printf("child process1 PID: %d\n", pid);
        printf("parent porcess PID: %d\n", pid1);
        wait(NULL);//wait until child ends
    }

    return 0;
}