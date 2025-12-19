CC = gcc
CFLAGS = -Wall -O2
LDFLAGS =

SRCS = blocking_echo.c nonblocking_echo.c select_echo.c poll_echo.c epoll_echo.c sigio_echo.c aio_read.c io_uring_echo.c
OBJS = $(SRCS:.c=.o)

all: blocking_echo nonblocking_echo select_echo poll_echo epoll_echo sigio_echo aio_read io_uring_echo

blocking_echo: blocking_echo.c
	$(CC) $(CFLAGS) -o $@ $<

nonblocking_echo: nonblocking_echo.c
	$(CC) $(CFLAGS) -o $@ $<

select_echo: select_echo.c
	$(CC) $(CFLAGS) -o $@ $<

poll_echo: poll_echo.c
	$(CC) $(CFLAGS) -o $@ $<

epoll_echo: epoll_echo.c
	$(CC) $(CFLAGS) -o $@ $<

sigio_echo: sigio_echo.c
	$(CC) $(CFLAGS) -o $@ $<

aio_read: aio_read.c -lrt
	$(CC) $(CFLAGS) -o $@ aio_read.c -lrt

io_uring_echo: io_uring_echo.c
	$(CC) $(CFLAGS) -o $@ io_uring_echo.c -luring

clean:
	rm -f $(OBJS) blocking_echo nonblocking_echo select_echo poll_echo epoll_echo sigio_echo aio_read

.PHONY: all clean
