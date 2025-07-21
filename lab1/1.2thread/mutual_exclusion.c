#include <stdio.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#define N 100000

int value = 0;
sem_t mutex; 
void *runner1(void *param);
void *runner2(void *param);

int main()
{
    pthread_t tid1, tid2;
    pthread_attr_t attr1, attr2;

    pthread_attr_init(&attr1);
    pthread_attr_init(&attr2);

    sem_init(&mutex, 0, 1);

    pthread_create(&tid1, &attr1, runner1, NULL);
    pthread_create(&tid2, &attr2, runner2, NULL);

    pthread_join(tid1,NULL);
    pthread_join(tid2, NULL);
      
    sem_destroy(&mutex);

    printf("value = %d\n", value);
}

void *runner1(void *param)
{
    printf("thread1 create success!\n");

    for (int i = 0; i < N; i++) {
        sem_wait(&mutex);
        value += 2;
        sem_post(&mutex);
    }    
}

void *runner2(void *param)
{
    printf("thread2 create success!\n");

    for (int i = 0; i < N; i++) {
        sem_wait(&mutex); 
        value -= 2;
        sem_post(&mutex);
    }
}