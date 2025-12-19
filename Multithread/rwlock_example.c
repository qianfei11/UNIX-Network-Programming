#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int shared = 0;
static pthread_rwlock_t rw = PTHREAD_RWLOCK_INITIALIZER;

void *reader(void *arg) {
  int id = (int)(long)arg;
  for (int i = 0; i < 5; i++) {
    pthread_rwlock_rdlock(&rw);
    printf("reader %d sees %d\n", id, shared);
    pthread_rwlock_unlock(&rw);
    usleep(100000);
  }
  return NULL;
}

void *writer(void *arg) {
  (void)arg;
  for (int i = 0; i < 5; i++) {
    pthread_rwlock_wrlock(&rw);
    shared++;
    printf("writer incremented to %d\n", shared);
    pthread_rwlock_unlock(&rw);
    usleep(200000);
  }
  return NULL;
}

int main(void) {
  pthread_t r1, r2, w;
  pthread_create(&r1, NULL, reader, (void *)1);
  pthread_create(&r2, NULL, reader, (void *)2);
  pthread_create(&w, NULL, writer, NULL);
  pthread_join(r1, NULL);
  pthread_join(r2, NULL);
  pthread_join(w, NULL);
  return 0;
}
