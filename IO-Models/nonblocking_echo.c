#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int ret;

  if (argc < 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    return 1;
  }
  int port = atoi(argv[1]);
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    perror("socket");
    return 1;
  }
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);
  int opt = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
    return 1;
  }
  if (listen(listenfd, 16) < 0) {
    perror("listen");
    return 1;
  }

  printf("Non-blocking demo server (single client) on %d\n", port);
  int c = accept(listenfd, NULL, NULL);
  if (c < 0) {
    perror("accept");
    return 1;
  }
  int flags = fcntl(c, F_GETFL, 0);
  fcntl(c, F_SETFL, flags | O_NONBLOCK);

  char buf[4096];
  for (;;) {
    ssize_t n = read(c, buf, sizeof(buf));
    if (n > 0) {
      ret = write(STDOUT_FILENO, "received: ", 10);
      if (ret < 0) {
        perror("write");
        break;
      }
      ret = write(STDOUT_FILENO, buf, n);
      if (ret < 0) {
        perror("write");
        break;
      }
      ret = write(c, buf, n);
      if (ret < 0) {
        perror("write");
        break;
      }
    } else if (n == 0) {
      break; /* peer closed */
    } else {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        usleep(100000);
        continue;
      } else {
        perror("read");
        break;
      }
    }
  }
  close(c);
  return 0;
}
