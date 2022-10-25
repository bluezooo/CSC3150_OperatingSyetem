#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "async.h"
#include "utlist.h"

threadpool_t *pool;
my_queue_t *queue;

void *worker(void *t);
void async_init(int num_threads)
{
    pthread_t threads[num_threads];
    pool = (threadpool_t *)malloc(sizeof(threadpool_t));
    queue = (my_queue_t *)malloc(sizeof(my_queue_t));
    queue->size = 0;
    pthread_mutex_init(&(pool->lock), NULL);
    pthread_cond_init(&(pool->cond), NULL);
    for (int i = 0; i < num_threads; i++)
    {
        pthread_create(&threads[i], NULL, &worker, NULL);
    }
    return;
}
void *worker(void *t)
{
    while (1)
    {
        pthread_mutex_lock(&(pool->lock));
        while (queue->size == 0)
        {
            pthread_cond_wait(&(pool->cond), &(pool->lock));
        }
        my_item_t *task = (my_item_t *)malloc(sizeof(my_item_t));
        task->function = queue->head->function;
        task->arg = queue->head->arg;
        DL_DELETE(queue->head, queue->head);
        queue->size--;
        pthread_mutex_unlock(&(pool->lock));
        (*(task->function))(task->arg);
    }
    pthread_exit(NULL);
}
void async_run(void(*handler), int args)
{
    pthread_mutex_lock(&(pool->lock));
    my_item_t *item = (my_item_t *)malloc(sizeof(my_item_t));
    item->function = handler;
    item->arg = args;
    DL_APPEND(queue->head, item);
    queue->size++;
    pthread_cond_signal(&(pool->cond));
    pthread_mutex_unlock(&(pool->lock));
}