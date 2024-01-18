#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define N 5 // 哲学家数量
#define LEFT (i + N - 1) % N // 左邻居的编号
#define RIGHT (i + 1) % N // 右邻居的编号
#define THINKING 0 // 哲学家思考状态
#define HUNGRY 1 // 哲学家饥饿状态
#define EATING 2 // 哲学家就餐状态

int state[N]; // 每个哲学家的状态
pthread_mutex_t mutex; // 互斥锁
pthread_cond_t cond[N]; // 条件变量，表示是否拿到了餐具

void test(int i);
void take_forks(int i);
void put_forks(int i);
void philosopher(int i);

int main()
{
    int i;
    pthread_t tid[N]; // 线程数组
    pthread_mutex_init(&mutex, NULL); // 初始化互斥锁
    for (i = 0; i < N; i++) // 初始化条件变量数组
        pthread_cond_init(&cond[i], NULL);
    for (i = 0; i < N; i++) // 创建N个哲学家线程
        pthread_create(&tid[i], NULL, (void*) philosopher, (void*) (long) i);
    for (i = 0; i < N; i++) // 等待线程结束
        pthread_join(tid[i], NULL);
    pthread_mutex_destroy(&mutex); // 销毁互斥锁
    for (i = 0; i < N; i++) // 销毁条件变量数组
        pthread_cond_destroy(&cond[i]);
    return 0;
}

// 测试目前是否可以得到两只叉子
void test(int i)
{
    if (state[i] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING)
    {
        state[i] = EATING;
        printf("Philosopher %d is eating with forks %d and %d\n", i, LEFT, i);
        pthread_cond_signal(&cond[i]); // 发信号，表明可以拿到叉子了
    }
}

// 请求两只叉子
void take_forks(int i)
{
    pthread_mutex_lock(&mutex); // 获取互斥锁
    state[i] = HUNGRY; // 改变自己的状态为饥饿
    printf("Philosopher %d is hungry\n", i);
    test(i); // 尝试拿叉子
    while (state[i] != EATING) // 如果没拿到，就等待条件变量发生变化
        pthread_cond_wait(&cond[i], &mutex);
    pthread_mutex_unlock(&mutex); // 放弃互斥锁
}

// 放下两只叉子
void put_forks(int i)
{
    pthread_mutex_lock(&mutex); // 获取互斥锁
    state[i] = THINKING; // 改变自己的状态为思考
    printf("Philosopher %d is now thinking\n", i);
    test(LEFT); // 可能左右邻居都可以吃，所以要分别尝试
    test(RIGHT);
    pthread_mutex_unlock(&mutex); // 放弃互斥锁
}

// 哲学家线程函数
void philosopher(int i)
{
    while (1)
    {
        printf("Philosopher %d is now thinking\n", i); // 哲学家思考
        sleep(random() % 5); // 随机等待
        take_forks(i); // 请求两只叉子
        printf("Philosopher %d starts eating\n", i);
        sleep(random() % 5); // 随机等待
        put_forks(i); // 放下两只叉子
    }
}
