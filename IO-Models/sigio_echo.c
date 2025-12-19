#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static int sock = -1;

void handler(int sig) {
  (void)sig;
  if (sock < 0)
    return;
  char buf[4096];
  struct sockaddr_in src;
  socklen_t sl = sizeof(src);
  ssize_t n =
      recvfrom(sock, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&src, &sl);
  if (n > 0) {
    buf[n] = 0;
    printf("SIGIO handler got %zd bytes: %s\n", n, buf);
  } else if (n < 0 && errno != EAGAIN)
    perror("recvfrom");
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    return 1;
  }
  int port = atoi(argv[1]);
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("socket");
    return 1;
  }
  struct sockaddr_in a = {0};
  a.sin_family = AF_INET;
  a.sin_addr.s_addr = INADDR_ANY;
  a.sin_port = htons(port);
  if (bind(sock, (struct sockaddr *)&a, sizeof(a)) < 0) {
    perror("bind");
    return 1;
  }

  signal(SIGIO, handler);
  fcntl(sock, F_SETOWN, getpid());
  int flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL, flags | O_ASYNC | O_NONBLOCK);

  printf("SIGIO UDP demo on port %d - send UDP datagrams to trigger SIGIO\n",
         port);
  /* keep process alive */
  for (;;)
    pause();
  return 0;
}
