#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(){
    int pidpid = getpid();
    printf("System_call PID: %d\n", pidpid);
    return 0;
}