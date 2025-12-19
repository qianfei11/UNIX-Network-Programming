#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXEVENTS 64

static int set_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1)
    return -1;
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static int create_and_bind(int port) {
  int sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd < 0)
    return -1;
  int opt = 1;
  setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(sfd);
    return -1;
  }
  return sfd;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    return 1;
  }
  int port = atoi(argv[1]);
  int listenfd = create_and_bind(port);
  if (listenfd < 0) {
    perror("bind");
    return 1;
  }
  if (listen(listenfd, 128) < 0) {
    perror("listen");
    close(listenfd);
    return 1;
  }
  set_nonblocking(listenfd);

  int efd = epoll_create1(0);
  if (efd < 0) {
    perror("epoll_create1");
    close(listenfd);
    return 1;
  }
  struct epoll_event ev, events[MAXEVENTS];
  ev.events = EPOLLIN;
  ev.data.fd = listenfd;
  epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &ev);

  printf("Reactor (epoll) echo server listening on %d\n", port);

  for (;;) {
    int n = epoll_wait(efd, events, MAXEVENTS, -1);
    if (n < 0) {
      perror("epoll_wait");
      break;
    }
    for (int i = 0; i < n; i++) {
      int fd = events[i].data.fd;
      if (fd == listenfd) {
        /* accept loop */
        for (;;) {
          int c = accept(listenfd, NULL, NULL);
          if (c < 0)
            break;
          set_nonblocking(c);
          struct epoll_event cev = {.events = EPOLLIN | EPOLLRDHUP,
                                    .data = {.fd = c}};
          epoll_ctl(efd, EPOLL_CTL_ADD, c, &cev);
        }
      } else {
        if (events[i].events & (EPOLLIN)) {
          char buf[4096];
          ssize_t r = read(fd, buf, sizeof(buf));
          if (r <= 0) {
            /* closed or error */
            epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
            close(fd);
          } else {
            /* echo */
            ssize_t w = 0;
            while (w < r)
              w += write(fd, buf + w, r - w);
          }
        }
        if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
          epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
          close(fd);
        }
      }
    }
  }

  close(listenfd);
  close(efd);
  return 0;
}
