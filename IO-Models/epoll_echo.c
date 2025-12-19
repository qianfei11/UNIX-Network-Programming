#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXEVENTS 64

int main(int argc, char **argv) {
  int ret;

  if (argc < 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    return 1;
  }
  int port = atoi(argv[1]);
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);
  int opt = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  ret = bind(listenfd, (struct sockaddr *)&addr, sizeof(addr));
  if (ret < 0) {
    perror("bind");
    close(listenfd);
    return 1;
  }
  listen(listenfd, 16);

  int efd = epoll_create1(0);
  struct epoll_event ev, events[MAXEVENTS];
  ev.events = EPOLLIN;
  ev.data.fd = listenfd;
  epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &ev);

  printf("epoll-based echo on %d\n", port);
  for (;;) {
    int n = epoll_wait(efd, events, MAXEVENTS, -1);
    for (int i = 0; i < n; i++) {
      int fd = events[i].data.fd;
      if (fd == listenfd) {
        int c = accept(listenfd, NULL, NULL);
        if (c < 0)
          continue;
        struct epoll_event ev2 = {.events = EPOLLIN, .data = {.fd = c}};
        epoll_ctl(efd, EPOLL_CTL_ADD, c, &ev2);
      } else {
        char buf[4096];
        ssize_t r = read(fd, buf, sizeof(buf));
        if (r <= 0) {
          epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
          close(fd);
        } else
          ret = write(fd, buf, r);
      }
    }
  }
  return 0;
}
