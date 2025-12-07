target = ipc-pipe ipc-fifo_read ipc-fifo_write ipc-msg_rcv ipc-msg_snd ipc-sem ipc-shm_read ipc-shm_write

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

ipc-msg_rcv: ipc-msg_rcv.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

ipc-msg_snd: ipc-msg_snd.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

ipc-sem: ipc-sem.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

ipc-shm_read: ipc-shm_read.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

ipc-shm_write: ipc-shm_write.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(target)
