#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
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
  char *p = "Hello, World!\n";

  signal(SIGINT, signal_handler);

  while (running) {
    sleep(1);
    write(STDOUT_FILENO, p, strlen(p) + 1);
  }

  return 0;
}
