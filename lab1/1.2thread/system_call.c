#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(){
    pid_t pidpid = getpid();
    printf("System_call PID: %d\n", pidpid);
    return 0;
}