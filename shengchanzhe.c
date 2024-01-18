#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>   // 实现多线程的头文件
#include <semaphore.h> // 实现信号量定义的头文件 
 
#define producerNumber 3   // 生产者的数目
#define consumerNumber 4   // 消费者的数目
#define M 6 // 缓冲区数目
 
int in = 0;   // 生产者放置产品的位置
int out = 0; // 消费者取产品的位置
 
int buff[M] = {0}; // 缓冲初始化为0， 开始时没有产品
 
sem_t empty_sem; // 信号量的数据类型为结构sem_t，它本质上是一个长整型的数,同步信号量， 当满了时阻止生产者放产品
sem_t full_sem;   // 同步信号量， 当没产品时阻止消费者消费
pthread_mutex_t mutex; // 互斥信号量， 一次只有一个线程访问缓冲
 
int producer_id = 0;   // 生产者id
int consumer_id = 0; // 消费者id
 
/* 打印缓冲情况 */
void print()
{
int i;
for(i = 0; i < M; i++)
   printf("%d ", buff[i]);
printf("\n");
}
 
/* 生产者方法 */
void *product()
{
int id = ++producer_id;
int count=30;
while(count--)
{
   // 用sleep的数量可以调节生产和消费的速度，便于观察
   sleep(1);
  
   sem_wait(&empty_sem); // 满不放
   pthread_mutex_lock(&mutex); // 实现互斥
  
   in = in % M;
   buff[in] = rand()%10;
   printf("生产者%d向%d号缓冲区放入了一个数据%d: \t", id, in, buff[in]);  
   print();  
   ++in;
  
   pthread_mutex_unlock(&mutex);
   sem_post(&full_sem);
}
}
 
/* 消费者方法 */
void *prochase()
{
int id = ++consumer_id;
int count=30;
while(count--)
{
   sleep(1);
  
   sem_wait(&full_sem); // 空不取
   pthread_mutex_lock(&mutex); // 实现互斥
  
   out = out % M;
   printf("消费者%d从%d号缓冲区取出一个数据%d: \t", id, out, buff[out]);
  
   buff[out] = 0;
   print();
   ++out;
  
   pthread_mutex_unlock(&mutex);
   sem_post(&empty_sem);
}
}
 
int main()
{
pthread_t id1[producerNumber]; // 声明生产者线程的ID数组
pthread_t id2[consumerNumber]; // 声明消费者线程的ID数组
int i;
int ret1[producerNumber];
int ret2[consumerNumber];
 
// 初始化同步信号量
int ini1 = sem_init(&empty_sem, 0, M); 
int ini2 = sem_init(&full_sem, 0, 0); // 同上初始化的描述
 
//初始化互斥信号量的函数
int ini3 = pthread_mutex_init(&mutex, NULL); 
 
// 创建producerNumber个生产者线程
for(i = 0; i < producerNumber; i++)
{
   ret1[i] = pthread_create(&id1[i], NULL, product, NULL);
}
//创建consumerNumber个消费者线程
for(i = 0; i < consumerNumber; i++)
{
   ret2[i] = pthread_create(&id2[i], NULL, prochase, NULL);
}
//销毁线程
for(i = 0; i < producerNumber; i++)
{
   pthread_join(id1[i],NULL);
   //pthread_join()函数来使主线程阻塞以等待其他线程退出
}
 
for(i = 0; i < consumerNumber; i++)
{
  pthread_join(id2[i],NULL);
}
 
exit(0);
}