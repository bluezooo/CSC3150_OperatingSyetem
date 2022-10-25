#ifndef __ASYNC__
#define __ASYNC__

#include <pthread.h>

typedef struct my_item
{
  struct my_item *next;
  struct my_item *prev;
  void (*function)(int);
  int arg;
} my_item_t;

typedef struct my_queue
{
  int size;
  my_item_t *head;
} my_queue_t;

typedef struct threadpool
{
  pthread_mutex_t lock;
  pthread_cond_t cond;
  pthread_t *threads;
  int queue_max_size;
} threadpool_t;

void async_init(int);
void async_run(void(*fx), int args);

#endif
