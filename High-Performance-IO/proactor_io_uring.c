#include <arpa/inet.h>
#include <fcntl.h>
#include <liburing.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define QD 256
#define BUF_SIZE 4096

struct conn {
  int fd;
  char *buf;
};

static int setup_listen(int port) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0)
    return -1;
  int opt = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(fd);
    return -1;
  }
  if (listen(fd, 128) < 0) {
    close(fd);
    return -1;
  }
  return fd;
}

static void make_nonblocking(int fd) {
  int f = fcntl(fd, F_GETFL, 0);
  if (f >= 0)
    fcntl(fd, F_SETFL, f | O_NONBLOCK);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    return 1;
  }
  int port = atoi(argv[1]);
  int listen_fd = setup_listen(port);
  if (listen_fd < 0) {
    perror("setup_listen");
    return 1;
  }

  struct io_uring ring;
  if (io_uring_queue_init(QD, &ring, 0) < 0) {
    perror("io_uring_queue_init");
    close(listen_fd);
    return 1;
  }

  printf("Proactor (io_uring) echo server on %d\n", port);

  /* submit initial accepts */
  for (int i = 0; i < 64; i++) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    if (!sqe)
      break;
    struct sockaddr_in *sa = malloc(sizeof(*sa));
    socklen_t *sl = malloc(sizeof(socklen_t));
    *sl = sizeof(*sa);
    io_uring_prep_accept(sqe, listen_fd, (struct sockaddr *)sa, sl, 0);
    io_uring_sqe_set_data(sqe, sa); /* tag accept using pointer */
  }
  io_uring_submit(&ring);

  for (;;) {
    struct io_uring_cqe *cqe;
    if (io_uring_wait_cqe(&ring, &cqe) < 0)
      continue;
    void *data = io_uring_cqe_get_data(cqe);
    int res = cqe->res;

    /* If data points to sockaddr, it's an accept completion */
    if (data && res >= 0) {
      struct sockaddr_in *sa = (struct sockaddr_in *)data;
      int client_fd = res;
      free(sa);
      make_nonblocking(client_fd);
      struct conn *c = malloc(sizeof(*c));
      if (!c) {
        close(client_fd);
        goto seen;
      }
      c->fd = client_fd;
      c->buf = malloc(BUF_SIZE);
      if (!c->buf) {
        close(client_fd);
        free(c);
        goto seen;
      }

      /* submit initial read on client */
      struct io_uring_sqe *rsqe = io_uring_get_sqe(&ring);
      io_uring_prep_recv(rsqe, c->fd, c->buf, BUF_SIZE, 0);
      io_uring_sqe_set_data(rsqe, c);
      io_uring_submit(&ring);

      /* re-arm another accept */
      struct io_uring_sqe *asqe = io_uring_get_sqe(&ring);
      struct sockaddr_in *nsa = malloc(sizeof(*nsa));
      socklen_t *nsl = malloc(sizeof(socklen_t));
      *nsl = sizeof(*nsa);
      io_uring_prep_accept(asqe, listen_fd, (struct sockaddr *)nsa, nsl, 0);
      io_uring_sqe_set_data(asqe, nsa);
      io_uring_submit(&ring);
    } else if (data && res < 0) {
      /* accept or op failed; free data if it was sockaddr */
      free(data);
    } else if (data == NULL) {
      /* shouldn't happen */
    } else {
      /* completion for client I/O: data is struct conn* */
      struct conn *c = (struct conn *)data;
      if (res <= 0) {
        close(c->fd);
        free(c->buf);
        free(c);
        goto seen;
      }
      int n = res;
      /* submit send to echo back */
      struct io_uring_sqe *wsqe = io_uring_get_sqe(&ring);
      io_uring_prep_send(wsqe, c->fd, c->buf, n, 0);
      io_uring_sqe_set_data(wsqe, c);
      io_uring_submit(&ring);
    }
  seen:
    io_uring_cqe_seen(&ring, cqe);
  }

  io_uring_queue_exit(&ring);
  close(listen_fd);
  return 0;
}
