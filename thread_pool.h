#ifndef THREAD_POOL_H
#define THREAD_POOL_H

typedef struct ThreadPool ThreadPool;

ThreadPool* thread_pool_create(int min_num, int max_num, int queue_capacity);

int thread_pool_destroy(ThreadPool* thread_pool);

void thread_pool_add_task(ThreadPool* thread_pool, void(*function)(void*), void* arg);

int thread_pool_busy_num(ThreadPool* thread_pool);

int thread_pool_alive_num(ThreadPool* thread_pool);

void* manager(void* arg); 

void* worker(void* arg);

void thread_exit(ThreadPool* thread_pool);

#endif  // THREAD_POOL_H