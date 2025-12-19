#ifndef THREADPOOL_H
#define THREADPOOL_H

struct threadpool;
typedef void (*task_fn)(void *);

struct threadpool *threadpool_create(int n);
void threadpool_submit(struct threadpool *tp, task_fn fn, void *arg);
void threadpool_destroy(struct threadpool *tp);

#endif
