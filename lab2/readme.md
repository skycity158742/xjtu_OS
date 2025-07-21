# 实验二 进程通信与内存管理
## 1.实验概述  
2.1中，实现了进程的软中断通信，使用了系统调用`fork()`创建了两个子进程，再调用`signal()`让父进程捕捉键盘上发出的中断信号，并调用`kill()`向两个紫禁城发出16，17的中断信号，子进程接收到中断信号后终止。父进程调用`wait()`等待两个子进程终止后再终止。  
2.2中，实现了一个管道通信，在有锁和无锁的情况下分别由两个子进程向管道分别写入2000个字符，父进程从管道中读出最终结果，输出最红结果并比较。  
2.3中，实现了内存管理的基本模型，并分别实现了FF,BF,WF三种内存空间分配算法（可以由使用者选择）。  
## 2.代码描述
### 2.1 进程的软中断通信
在父进程中，创建了两个子进程，其进程标识分别为`pid1`和`pid2`。父进程通过`signal()`系统调用等待键盘输入信号`SIGINT`,`SIGQUIT`或`SIGALRM`，调用`inter_handler()`判断中断信号类型，并设置信号旗帜`flag`。在两个子进程中，分别使用`signal()`系统调用等待16，17两个软中断代码，并分别调用`child1_handler()`和`child2_handler()`函数以实现两个进程对应的输出和自我中断。在父进程中，使用`wait()`系统调用来等待两个子进程的终止，之后自我终止。  
```C

// softstop.c

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

```  


### 2.2 进程的管道通信
在2.2中，定义了一个int类型数组`int fd[2]`，并将其与管道的读写端关联。之后定义一个长度为5000的缓冲区`char InPipe[5000]`。之后在子进程1中向管道的写端写入2000个字符1并在子进程2中向管道写入2000个字符2。在写之前，采用`lockf()`方法对管道上锁，在写操作完成之后，采用同样的方法对管道解锁，即可实现两个进程的互斥写入，并在父进程中一次性读出4000个字符。
```C

// pipe.c


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

```
### 2.3 内存分配与管理
```C
// memory_allocation.c


/*
**************************************************
    main function:
    1 - set memory size (default = 1024)
    2 - select memory allocation algorithm
    3 - New process
    4 - Terminate a process
    5 - Display memory usage
    0 - Exit
**************************************************
*/


#include <stdio.h>
#include <stdlib.h>

#define MEM_H 
#define PROCESS_NAME_LEN 32   /*进程名长度*/
#define PROCESS_NAME_LEN 32   /*进程名长度*/
#define MIN_SLICE 10          /*最小碎片的大小*/
#define DEFAULT_MEM_SIZE 1024 /*内存大小*/
#define DEFAULT_MEM_START 0   /*起始位置*/
#define MA_FF 1
#define MA_BF 2
#define MA_WF 3

struct free_block_type *init_free_block(int mem_size);
int set_mem_size();
void set_algorithm();
int new_process();
void kill_process();
int display_mem_usage();
void do_exit();

void rearrange(int algorithm);
int allocate_mem(struct allocated_block*);
struct allocated_block *find_process(int );
int free_mem(struct allocated_block*a);
int dispose(struct allocated_block*);
void rearrange_FF();
void rearrange_BF();
void rearrange_WF();

void display_menu();

/*描述每一个空闲块的数据结构*/
struct free_block_type{
    int size;
    int start_addr;
    struct free_block_type *next;
};
/*指向内存中空闲块链表的首指针*/
struct free_block_type *free_block;

/*每个进程分配到的内存块的描述*/
struct allocated_block{
    int pid;
    int size;
    int start_addr;
    char process_name[PROCESS_NAME_LEN];
    struct allocated_block *next;
};
/*进程分配内存块链表的首指针*/
struct allocated_block *allocated_block_head = NULL;



int mem_size = DEFAULT_MEM_SIZE; /*内存大小*/
int ma_algorithm = MA_FF;        /*当前分配算法*/
static int pid = 0;              /*初始 pid*/
int flag = 0;                    /*设置内存大小标志*/




int main()
{
    char choice;
    pid = 0;
    free_block = init_free_block(mem_size); // 初始化空闲区
    while(1) {
        display_menu(); //显示菜单
        fflush(stdin);
        choice = getchar(); // 获取用户输入
        switch(choice){
            case '1': set_mem_size(); break;                //设置内存大小
            case '2': set_algorithm(); flag = 1; break;     // 设置算法
            case '3': new_process(); flag = 1; break;       // 创建新进程
            case '4': kill_process(); flag = 1; break;      // 删除进程
            case '5': display_mem_usage(); flag =1 ; break; // 显示内存使用
            case '0': do_exit(); exit(0);                   //释放链表并退出
            default: break; 
        } 
    }
}

/*初始化空闲块，默认为一块，可以指定大小及起始地址*/
struct free_block_type* init_free_block(int mem_size){
    struct free_block_type *fb;
    fb = (struct free_block_type *)malloc(sizeof(struct free_block_type));
    if (fb == NULL)
    {
        printf("No mem\n");
        return NULL;
    }
    fb->size = mem_size;
    fb->start_addr = DEFAULT_MEM_START;
    fb->next = NULL;
    return fb;
}

/*显示菜单*/
void display_menu(){
    printf("\n");
    printf("1 - Set memory size (default=%d)\n", DEFAULT_MEM_SIZE);
    printf("2 - Select memory allocation algorithm\n");
    printf("3 - New process \n");
    printf("4 - Terminate a process \n");
    printf("5 - Display memory usage \n");
    printf("0 - Exit\n");
}

/*设置内存的大小*/
int set_mem_size(){
    int size;
    if (flag != 0)
    { // 防止重复设置
        printf("Cannot set memory size again\n");
        return 0;
    }
    printf("Total memory size =");
    scanf("%d", &size);
    if (size > 0)
    {
        mem_size = size;
        free_block->size = mem_size;
    }
    flag = 1;
    return 1;
}

/* 设置当前的分配算法 */
void set_algorithm(){
    int algorithm;
    printf("\t1 - First Fit\n");
    printf("\t2 - Best Fit \n");
    printf("\t3 - Worst Fit \n");
    scanf("%d", &algorithm);
    if (algorithm >= 1 && algorithm <= 3)
        ma_algorithm = algorithm;
    // 按指定算法重新排列空闲区链表
    rearrange(ma_algorithm);
}

/*按指定的算法整理内存空闲块链表*/
void rearrange(int algorithm){  
    switch(algorithm){
        case MA_FF: rearrange_FF(); break;
        case MA_BF: rearrange_BF(); break;
        case MA_WF: rearrange_WF(); break;
    }
}

/*按 FF 算法重新整理内存空闲块链表*/
void rearrange_FF(){
    struct free_block_type *fbt,*work, *minblock;
    work = free_block;
    if (work->next == NULL)
        return;
   
    // Sort by starting address
    while (work)
    {
        fbt = work;
        int min = 0x7FFFFFFF;
        while (fbt){
            if (fbt->start_addr < min)
            {
                min = fbt->start_addr;
                minblock = fbt;
            }
            fbt = fbt->next;
        }
        int temp_size, temp_addr;
        temp_size = work->size;
        temp_addr = work->start_addr;
        work->size = minblock->size;
        work->start_addr = minblock->start_addr;
        minblock->size = temp_size;
        minblock->start_addr = temp_addr;
        work = work->next;
    }
}

/*按 BF 算法重新整理内存空闲块链表*/
void rearrange_BF(){
    struct free_block_type *fbt,*work, *minblock;
    work = free_block;
    if (work->next == NULL)
        return;
    
    // Sort from smallest to largest based on memory size
    while (work)
    {
        fbt = work;
        int min = 0x7FFFFFFF;
        while (fbt){
            if (fbt->size < min)
            {
                min = fbt->size;
                minblock = fbt;
            }
            fbt = fbt->next;
        }
        int temp_size, temp_addr;
        temp_size = work->size;
        temp_addr = work->start_addr;
        work->size = minblock->size;
        work->start_addr = minblock->start_addr;
        minblock->size = temp_size;
        minblock->start_addr = temp_addr;
        work = work->next;
    }
}

/*按 WF 算法重新整理内存空闲块链表*/
void rearrange_WF(){
    struct free_block_type *fbt,*work, *maxblock;
    work = free_block;
    if (work->next == NULL)
        return;
    
    // Sort from smallest to largest based on memory size
    while (work)
    {
        fbt = work;
        int max = -1;
        while (fbt){
            if (fbt->size > max)
            {
                max = fbt->size;
                maxblock = fbt;
            }
            fbt = fbt->next;
        }
        int temp_size, temp_addr;
        temp_size = work->size;
        temp_addr = work->start_addr;
        work->size = maxblock->size;
        work->start_addr = maxblock->start_addr;
        maxblock->size = temp_size;
        maxblock->start_addr = temp_addr;
        work = work->next;
    }
}

/*创建新的进程，主要是获取内存的申请数量*/
int new_process(){
    struct allocated_block *ab;
    int size;
    int ret;
    ab = (struct allocated_block *)malloc(sizeof(struct allocated_block));
    if (!ab) exit(-5);
    ab->next = NULL;
    pid++;
    sprintf(ab->process_name, "PROCESS-%02d", pid);
    ab->pid = pid;
    printf("Memory for %s:", ab->process_name);
    scanf("%d", &size);
    if (size > 0)
        ab->size = size;
    ret = allocate_mem(ab); /* 从空闲区分配内存， ret==1 表示分配 ok*/
    /*如果此时 allocated_block_head 尚未赋值，则赋值*/
    if ((ret == 1) && (allocated_block_head == NULL))
    {
        allocated_block_head = ab;
        return 1;
    }
    /*分配成功，将该已分配块的描述插入已分配链表*/
    else if (ret == 1){
        ab->next = allocated_block_head;
        allocated_block_head = ab;
        return 2;
    }
    else if(ret == -1){ /*分配不成功*/
        printf("Allocation fail\n");
        free(ab);
        return -1;
    }
    return 3;
}

/*分配内存模块*/
int allocate_mem(struct allocated_block *ab){
    struct free_block_type *fbt, *pre;
    int request_size = ab->size;
    int free_sum = 0; // sum the size of all the free blocks
    fbt = pre = free_block;
    while (fbt) // traverse the free block link list
    {
        if(fbt == NULL)
            return -1;
        free_sum += fbt->size; // get the present sum
        if (fbt->size >= request_size)
        {   // allocate the block
            ab->start_addr = fbt->start_addr;
            
            // cut free block
            if(fbt->size == request_size){ // exactly the size requested
                if(pre == fbt && fbt->next == NULL){ // if is the first and the only free block
                    fbt->size = 0;
                }
                else if(pre == fbt && fbt->next){ // if is the first but not the only free block
                    free_block = fbt->next;
                    free(fbt);
                }
                else{ // neither the first nor the only free block
                    pre->next = fbt->next;
                    free(fbt);
                }
            }
            else{  // bigger than requested, modify the size and the start_addr
                fbt->start_addr += request_size;
                fbt->size -= request_size;
            }
            rearrange(ma_algorithm);
            return 1; // if free block found, function should end here
        }
        pre = fbt;
        fbt = fbt->next;
    }
    
    if(free_sum >= request_size){ // not found, do memory compression
        struct allocated_block *head = allocated_block_head;
        struct free_block_type *fbt_n, *pre_n;
        fbt_n = pre_n = free_block;
        int addr_new = 0;

        while (head)
        {
            head->start_addr = addr_new;
            addr_new += head->size;
            head = head->next;
        }
        ab->start_addr = addr_new;
        addr_new += request_size;

        // There must be over one free block
        fbt_n = fbt_n->next;
        while (fbt_n)
        {
            pre_n = fbt_n;
            fbt_n = fbt_n->next;
            free(pre_n);
        }
        free_block->start_addr = addr_new;
        free_block->size = mem_size - addr_new;
        free_block->next = NULL;

        rearrange(ma_algorithm);
        return 1;
    }
    return -1;
}

/*删除进程，归还分配的存储空间，并删除描述该进程内存分配的节点*/
void kill_process(){
    struct allocated_block *ab;
    int pid;
    printf("Kill Process, pid=");
    scanf("%d", &pid);
    ab = find_process(pid);
   
    if (ab != NULL)
    {
        free_mem(ab); /*释放 ab 所表示的分配区*/
        dispose(ab);  /*释放 ab 数据结构节点*/
    }
}

struct allocated_block *find_process(int pid){
    struct allocated_block *ab = allocated_block_head;
    while(ab){
        if(ab->pid == pid) 
            return ab;
        ab = ab->next;
    }
    return NULL;
}

/*将 ab 所表示的已分配区归还，并进行可能的合并*/
int free_mem(struct allocated_block *ab){
    int algorithm = ma_algorithm;
    struct free_block_type *fbt, *pre, *work;
    fbt = (struct free_block_type *)malloc(sizeof(struct free_block_type));
    if (!fbt)
        return -1;
    fbt->start_addr = ab->start_addr;
    fbt->size = ab->size;

    // Put new block in the end
    work = free_block;
    if(work){
        while(work->next){
            work = work->next;
        }
        work->next = fbt;
    }
    else{
        free_block = fbt;
    }
    rearrange_FF();

    // Merge adjacent regions
    pre = free_block;
    work = free_block->next;
    while(work){
        if(pre->start_addr + pre->size == work->start_addr){
            pre->size += work->size;
            pre->next = work->next;
            free(work);
            work = pre->next;
        }
        else{
            pre = work;
            work = work->next;
        }
    }

    rearrange(ma_algorithm);
    return 1;
}

/*释放 ab 数据结构节点*/
int dispose(struct allocated_block *free_ab){
    struct allocated_block *pre, *ab;
    if (free_ab == allocated_block_head)
    { /*如果要释放第一个节点*/
        allocated_block_head = allocated_block_head->next;
        free(free_ab);
        return 1;
    }
    pre = allocated_block_head;
    ab = allocated_block_head->next;
    while (ab != free_ab)
    {
        pre = ab;
        ab = ab->next;
    }
    pre->next = ab->next;
    free(ab);
    return 2;
}

/* 显示当前内存的使用情况，包括空闲区的情况和已经分配的情况 */
int display_mem_usage(){
    struct free_block_type *fbt = free_block;
    struct allocated_block *ab = allocated_block_head;
    if (fbt == NULL)
        return (-1);
    printf("----------------------------------------------------------\n"); /* 显示空闲区 */
    printf("Free Memory:\n");
    printf("%20s %20s\n", " start_addr", " size");
    while (fbt != NULL)
    {
        printf("%20d %20d\n", fbt->start_addr, fbt->size);
        fbt = fbt->next;
    }
    /* 显示已分配区 */
    printf("\nUsed Memory:\n");
    printf("%10s %20s %10s %10s\n", "PID", "ProcessName", "start_addr", " size");
    while (ab != NULL)
    {
        printf("%10d %20s %10d %10d\n", ab->pid, ab->process_name,
               ab->start_addr, ab->size);
        ab = ab->next;
    }
    printf("----------------------------------------------------------\n");
    return 0;
}

void do_exit(){
    struct free_block_type *ftb, *del;
    struct allocated_block *alb, *dela;
    ftb = free_block;
    alb = allocated_block_head;
    while (ftb){
        del = ftb;
        ftb = ftb->next;
        free(del);
    }
    while(alb){
        dela = alb;
        alb = alb->next;
        free(dela);
    }
}
```