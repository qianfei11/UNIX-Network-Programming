#include "threadpool.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void task(void *arg) {
  int id = (int)(long)arg;
  printf("task %d running on thread %lu\n", id, (unsigned long)pthread_self());
  usleep(100000);
}

int main(void) {
  struct threadpool *tp = threadpool_create(4);
  for (long i = 0; i < 20; i++)
    threadpool_submit(tp, task, (void *)i);
  sleep(1);
  threadpool_destroy(tp);
  return 0;
}
