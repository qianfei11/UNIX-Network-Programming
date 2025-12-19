#include <arpa/inet.h>
#include <fcntl.h>
#include <liburing.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define QD 256
#define BUF_SIZE 4096

enum { OP_RECV = 1, OP_SEND = 2 };

struct conn {
  int fd;
  char *buf;
};

struct accept_req {
  struct sockaddr_storage addr;
  socklen_t addr_len;
};

struct io_op {
  struct conn *c;
  int type; /* OP_RECV or OP_SEND */
};

static int setup_listen(int port) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0)
    return -1;
  int opt = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);
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

static void submit_accept(struct io_uring *ring, int listen_fd) {
  struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
  if (!sqe)
    return;
  struct accept_req *ar = malloc(sizeof(*ar));
  if (!ar)
    return;
  ar->addr_len = sizeof(ar->addr);
  io_uring_prep_accept(sqe, listen_fd, (struct sockaddr *)&ar->addr,
                       &ar->addr_len, 0);
  /* mark accept pointer by setting low bit 1 */
  unsigned long tag = ((unsigned long)ar) | 1UL;
  io_uring_sqe_set_data(sqe, (void *)tag);
  io_uring_submit(ring);
}

static void submit_recv(struct io_uring *ring, struct conn *c) {
  struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
  if (!sqe)
    return;
  struct io_op *op = malloc(sizeof(*op));
  if (!op)
    return;
  op->c = c;
  op->type = OP_RECV;
  io_uring_prep_recv(sqe, c->fd, c->buf, BUF_SIZE, 0);
  io_uring_sqe_set_data(sqe, op);
  io_uring_submit(ring);
}

static void submit_send(struct io_uring *ring, struct conn *c, int len) {
  struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
  if (!sqe)
    return;
  struct io_op *op = malloc(sizeof(*op));
  if (!op)
    return;
  op->c = c;
  op->type = OP_SEND;
  io_uring_prep_send(sqe, c->fd, c->buf, len, 0);
  io_uring_sqe_set_data(sqe, op);
  io_uring_submit(ring);
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

  printf("io_uring concurrent echo server on %d\n", port);

  /* prime a few accepts */
  for (int i = 0; i < 32; i++)
    submit_accept(&ring, listen_fd);

  for (;;) {
    struct io_uring_cqe *cqe;
    if (io_uring_wait_cqe(&ring, &cqe) < 0)
      continue;
    void *ud = io_uring_cqe_get_data(cqe);
    int res = cqe->res;

    if (((unsigned long)ud & 1UL) == 1UL) {
      /* accept completion */
      struct accept_req *ar = (struct accept_req *)((unsigned long)ud & ~1UL);
      int client_fd = res;
      free(ar);
      /* immediately re-submit accept */
      submit_accept(&ring, listen_fd);

      if (client_fd < 0) {
        /* accept failed */
        io_uring_cqe_seen(&ring, cqe);
        continue;
      }
      make_nonblocking(client_fd);
      struct conn *c = malloc(sizeof(*c));
      if (!c) {
        close(client_fd);
        io_uring_cqe_seen(&ring, cqe);
        continue;
      }
      c->fd = client_fd;
      c->buf = malloc(BUF_SIZE);
      if (!c->buf) {
        close(client_fd);
        free(c);
        io_uring_cqe_seen(&ring, cqe);
        continue;
      }
      /* submit first recv for this client */
      submit_recv(&ring, c);
    } else {
      struct io_op *op = (struct io_op *)ud;
      struct conn *c = op->c;
      if (op->type == OP_RECV) {
        if (res <= 0) {
          /* closed or error */
          close(c->fd);
          free(c->buf);
          free(c);
          free(op);
          io_uring_cqe_seen(&ring, cqe);
          continue;
        }
        int n = res;
        /* got data, submit send */
        submit_send(&ring, c, n);
        free(op);
      } else if (op->type == OP_SEND) {
        if (res < 0) {
          /* send error, close */
          close(c->fd);
          free(c->buf);
          free(c);
          free(op);
          io_uring_cqe_seen(&ring, cqe);
          continue;
        }
        /* after send, re-submit recv to continue echoing */
        submit_recv(&ring, c);
        free(op);
      } else {
        free(op);
      }
    }

    io_uring_cqe_seen(&ring, cqe);
  }

  io_uring_queue_exit(&ring);
  close(listen_fd);
  return 0;
}
