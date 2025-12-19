#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXCLIENTS FD_SETSIZE

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

  fd_set allset, rset;
  FD_ZERO(&allset);
  FD_SET(listenfd, &allset);
  int maxfd = listenfd;
  int clients[MAXCLIENTS];
  for (int i = 0; i < MAXCLIENTS; i++)
    clients[i] = -1;

  printf("select-based echo on %d\n", port);
  for (;;) {
    rset = allset;
    int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
    if (FD_ISSET(listenfd, &rset)) {
      int c = accept(listenfd, NULL, NULL);
      int i;
      for (i = 0; i < MAXCLIENTS; i++)
        if (clients[i] < 0) {
          clients[i] = c;
          break;
        }
      if (i == MAXCLIENTS) {
        close(c);
      }
      FD_SET(c, &allset);
      if (c > maxfd)
        maxfd = c;
      if (--nready <= 0)
        continue;
    }
    for (int i = 0; i < MAXCLIENTS; i++) {
      int fd = clients[i];
      if (fd < 0)
        continue;
      if (FD_ISSET(fd, &rset)) {
        char buf[4096];
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n <= 0) {
          close(fd);
          FD_CLR(fd, &allset);
          clients[i] = -1;
        } else
          ret = write(fd, buf, n);
        if (--nready <= 0)
          break;
      }
    }
  }
  return 0;
}
