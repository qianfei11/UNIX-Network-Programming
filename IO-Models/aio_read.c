#include <aio.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int ret;

  if (argc < 2) {
    fprintf(stderr, "usage: %s <file>\n", argv[0]);
    return 1;
  }
  const char *path = argv[1];
  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  struct aiocb cb;
  memset(&cb, 0, sizeof(cb));
  char *buf = malloc(8192);
  cb.aio_fildes = fd;
  cb.aio_buf = buf;
  cb.aio_nbytes = 8191;
  cb.aio_offset = 0;

  if (aio_read(&cb) < 0) {
    perror("aio_read");
    close(fd);
    return 1;
  }

  const struct aiocb *list[1] = {&cb};
  if (aio_suspend(list, 1, NULL) < 0) {
    perror("aio_suspend");
  }

  int err = aio_error(&cb);
  ssize_t n = aio_return(&cb);
  if (err == 0 && n > 0) {
    buf[n] = 0;
    printf("AIO read %zd bytes:\n", n);
    ret = write(STDOUT_FILENO, buf, n);
    if (ret < 0) {
      perror("write");
    }
  } else if (n == 0) {
    printf("AIO read: EOF\n");
  } else {
    fprintf(stderr, "AIO error: %s\n", strerror(err));
  }

  free(buf);
  close(fd);
  return 0;
}
