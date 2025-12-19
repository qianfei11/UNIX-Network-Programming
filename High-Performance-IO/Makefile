CC = gcc
CFLAGS = -Wall -O2

all: reactor_epoll proactor_io_uring

reactor_epoll: reactor_epoll.c
	$(CC) $(CFLAGS) -o $@ $<

proactor_io_uring: proactor_io_uring.c
	$(CC) $(CFLAGS) -o $@ proactor_io_uring.c -luring

clean:
	rm -f reactor_epoll proactor_io_uring

.PHONY: all clean
