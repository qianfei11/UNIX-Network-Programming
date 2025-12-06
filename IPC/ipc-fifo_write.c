#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int running = 1;

void signal_handler(int sig) {
  if (sig == SIGINT) {
    running = 0;
    printf("\nReceived SIGINT (Ctrl+C), exiting gracefully...\n");
  }
}

int main(int argc, const char *argv[]) {
  int ret, r, fd;
  char *p = "hello,world";

  if (argc < 2) {
    printf("%s fifioname\n", argv[0]);
    exit(1);
  }

  signal(SIGINT, signal_handler);

  ret = access(argv[1], F_OK);
  if (ret == -1) {
    r = mkfifo(argv[1], 0664);
    if (r == -1) {
      perror("mkfifo");
      exit(1);
    }
  }

  fd = open(argv[1], O_WRONLY);
  if (fd == -1) {
    perror("open");
    exit(1);
  }

  while (running) {
    sleep(1);
    write(fd, p, strlen(p) + 1);
  }

  close(fd);
  printf("FIFO writer closed.\n");

  return 0;
}
