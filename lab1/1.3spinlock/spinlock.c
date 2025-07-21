#include <stdio.h>
#include <pthread.h>

/*定义自旋锁数据结构*/
typedef struct 
{
    int flag;
} spinlock_t;

/*初始化*/
void spinlock_init(spinlock_t *lock) 
{
    lock->flag = 0;
}

/*获取自旋锁*/
void spinlock_lock(spinlock_t *lock) 
{
    while (__sync_lock_test_and_set(&lock->flag, 1)) {
    // 自旋等待
    }
}

/*释放自旋锁*/
void spinlock_unlock(spinlock_t *lock) 
{
    __sync_lock_release(&lock->flag);
}

int shared_value = 0;

void *thread_function1(void *arg) 
{
    spinlock_t *lock = (spinlock_t *)arg;
    for (int i = 0; i < 5000; ++i) {
        spinlock_lock(lock);
        shared_value += 2;
        if(i == 1500) printf("中间值：%d\n",shared_value);
        spinlock_unlock(lock);
    }
    return NULL;
}

void *thread_function2(void *arg) 
{
    spinlock_t *lock = (spinlock_t *)arg;
    for (int i = 0; i < 5000; ++i) {
        spinlock_lock(lock);
        shared_value -= 2;
        spinlock_unlock(lock);
    }
    return NULL;
}

int main() 
{
    /*创建两个线程*/
    pthread_t thread1, thread2;
    
    /*初始化一个自旋锁*/
    spinlock_t lock;

    printf("Shared_value: %d\n",shared_value);

    /*初始化自旋锁*/
    spinlock_init(&lock);

    if (pthread_create(&thread1, NULL, thread_function1, &lock) != 0)
    {
        perror("pthread_create");
        return 1;
    }
    if (pthread_create(&thread2, NULL, thread_function2, &lock) != 0) 
    {
        perror("pthread_create");
        return 1;
    }

    /*等待线程结束*/
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    /*输出共享变量*/
    printf("Shared_value: %d\n",shared_value);
    return 0;
}