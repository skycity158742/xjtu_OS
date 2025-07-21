#include<stdio.h>
#include<sys/time.h>
#include<unistd.h>

int main()
{
    int a = 10, b = 26;
    int ret = syscall(96, a, b); 
    printf("%d\n", ret);
    return 0;
}