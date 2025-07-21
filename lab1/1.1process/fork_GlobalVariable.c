#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

/* Set the global variable */
int global_value;

int main()
{    
pid_t pid, pid1;
    global_value = 0;

    /* fork a child process */
    pid = fork();

    if(pid < 0){ /* error occured */
        fprintf(stderr, "Fork Failed!");
        return 1;
    }
    else if(pid == 0){ /* child process */
        pid1 = getpid();
        global_value--;
        /*
        printf("child: pid = %d/n", pid); //A, pid expected to be 0
        printf("child: pid1 = %d/n", pid1); //B, pid expected to be child's pid
         */
        printf("child: value = %d\n", global_value);
    }
    else{/* parent process */
        pid1 = getpid();
        global_value++;
        /*
        printf("parent: pid = %d/n", pid); //C, pid expected to be child's pid
        printf("parent: pid1 = %d/n", pid1); //D, pid1 expected to be parent's pid
        wait(NULL);  //wait until child ends
         */
        printf("parent: value = %d\n", global_value);
    }

    return 0;
}