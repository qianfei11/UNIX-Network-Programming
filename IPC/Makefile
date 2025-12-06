target = ipc-pipe ipc-fifo_read ipc-fifo_write

CC = gcc
LDFLAGS =
LDLIBS =
CFLAGS = -Wall -g

all: $(target)

ipc-pipe: ipc-pipe.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

ipc-fifo_read: ipc-fifo_read.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

ipc-fifo_write: ipc-fifo_write.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(target)
