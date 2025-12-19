CC = gcc
CFLAGS = -Wall -O2

all: mutex_example condvar_example rwlock_example threadpool_test

mutex_example: mutex_example.c
	$(CC) $(CFLAGS) -o $@ $< -lpthread

condvar_example: condvar_example.c
	$(CC) $(CFLAGS) -o $@ $< -lpthread

rwlock_example: rwlock_example.c
	$(CC) $(CFLAGS) -o $@ $< -lpthread

threadpool_test: threadpool.c threadpool_test.c
	$(CC) $(CFLAGS) -o $@ threadpool.c threadpool_test.c -lpthread

clean:
	rm -f mutex_example condvar_example rwlock_example threadpool_test

.PHONY: all clean
