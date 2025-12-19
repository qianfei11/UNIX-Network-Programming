#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
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

  printf("Blocking echo server listening on %d\n", port);
  for (;;) {
    int c = accept(listenfd, NULL, NULL);
    if (c < 0) {
      perror("accept");
      continue;
    }
    char buf[4096];
    ssize_t n;
    while ((n = read(c, buf, sizeof(buf))) > 0) {
      ssize_t w = 0;
      while (w < n)
        w += write(c, buf + w, n - w);
    }
    close(c);
  }
  return 0;
}
