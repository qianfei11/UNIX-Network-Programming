#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXCLIENTS 1024

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

  struct pollfd fds[MAXCLIENTS];
  for (int i = 0; i < MAXCLIENTS; i++) {
    fds[i].fd = -1;
    fds[i].events = 0;
  }
  fds[0].fd = listenfd;
  fds[0].events = POLLIN;

  printf("poll-based echo on %d\n", port);
  for (;;) {
    int n = poll(fds, MAXCLIENTS, -1);
    if (n <= 0)
      continue;
    if (fds[0].revents & POLLIN) {
      int c = accept(listenfd, NULL, NULL);
      for (int i = 1; i < MAXCLIENTS; i++)
        if (fds[i].fd < 0) {
          fds[i].fd = c;
          fds[i].events = POLLIN;
          break;
        }
    }
    for (int i = 1; i < MAXCLIENTS; i++) {
      if (fds[i].fd < 0)
        continue;
      if (fds[i].revents & POLLIN) {
        char buf[4096];
        ssize_t r = read(fds[i].fd, buf, sizeof(buf));
        if (r <= 0) {
          close(fds[i].fd);
          fds[i].fd = -1;
        } else
          ret = write(fds[i].fd, buf, r);
      }
    }
  }
  return 0;
}
