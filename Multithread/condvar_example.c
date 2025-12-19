#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int ready = 0;

void *producer(void *arg) {
  (void)arg;
  sleep(1);
  pthread_mutex_lock(&lock);
  ready = 1;
  printf("producer: ready=1, signalling\n");
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&lock);
  return NULL;
}

void *consumer(void *arg) {
  (void)arg;
  pthread_mutex_lock(&lock);
  while (!ready) {
    printf("consumer: waiting\n");
    pthread_cond_wait(&cond, &lock);
  }
  printf("consumer: got signal, ready=%d\n", ready);
  pthread_mutex_unlock(&lock);
  return NULL;
}

int main(void) {
  pthread_t p, c;
  pthread_create(&c, NULL, consumer, NULL);
  pthread_create(&p, NULL, producer, NULL);
  pthread_join(p, NULL);
  pthread_join(c, NULL);
  return 0;
}
