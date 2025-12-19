#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int counter = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *worker(void *arg) {
  int id = (int)(long)arg;
  for (int i = 0; i < 100000; i++) {
    pthread_mutex_lock(&lock);
    counter++;
    pthread_mutex_unlock(&lock);
  }
  printf("worker %d done\n", id);
  return NULL;
}

int main(void) {
  const int N = 4;
  pthread_t t[N];
  for (long i = 0; i < N; i++)
    pthread_create(&t[i], NULL, worker, (void *)i);
  for (int i = 0; i < N; i++)
    pthread_join(t[i], NULL);
  printf("counter = %d (expected %d)\n", counter, 100000 * N);
  return 0;
}
