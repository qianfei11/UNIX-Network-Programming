#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef void (*task_fn)(void *);

struct task {
  task_fn fn;
  void *arg;
  struct task *next;
};

struct threadpool {
  pthread_t *threads;
  int nthreads;
  struct task *head, *tail;
  pthread_mutex_t lock;
  pthread_cond_t cond;
  int stop;
};

static void *worker(void *p) {
  struct threadpool *tp = p;
  for (;;) {
    pthread_mutex_lock(&tp->lock);
    while (!tp->head && !tp->stop)
      pthread_cond_wait(&tp->cond, &tp->lock);
    if (tp->stop && !tp->head) {
      pthread_mutex_unlock(&tp->lock);
      break;
    }
    struct task *t = tp->head;
    tp->head = t->next;
    if (!tp->head)
      tp->tail = NULL;
    pthread_mutex_unlock(&tp->lock);
    t->fn(t->arg);
    free(t);
  }
  return NULL;
}

struct threadpool *threadpool_create(int n) {
  struct threadpool *tp = calloc(1, sizeof(*tp));
  tp->nthreads = n;
  tp->threads = calloc(n, sizeof(pthread_t));
  pthread_mutex_init(&tp->lock, NULL);
  pthread_cond_init(&tp->cond, NULL);
  for (int i = 0; i < n; i++)
    pthread_create(&tp->threads[i], NULL, worker, tp);
  return tp;
}

void threadpool_submit(struct threadpool *tp, task_fn fn, void *arg) {
  struct task *t = malloc(sizeof(*t));
  t->fn = fn;
  t->arg = arg;
  t->next = NULL;
  pthread_mutex_lock(&tp->lock);
  if (tp->tail)
    tp->tail->next = t;
  else
    tp->head = t;
  tp->tail = t;
  pthread_cond_signal(&tp->cond);
  pthread_mutex_unlock(&tp->lock);
}

void threadpool_destroy(struct threadpool *tp) {
  pthread_mutex_lock(&tp->lock);
  tp->stop = 1;
  pthread_cond_broadcast(&tp->cond);
  pthread_mutex_unlock(&tp->lock);
  for (int i = 0; i < tp->nthreads; i++)
    pthread_join(tp->threads[i], NULL);
  free(tp->threads);
  while (tp->head) {
    struct task *t = tp->head;
    tp->head = t->next;
    free(t);
  }
  pthread_mutex_destroy(&tp->lock);
  pthread_cond_destroy(&tp->cond);
  free(tp);
}
