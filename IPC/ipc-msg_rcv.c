#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <unistd.h>

#define MSG_ID 1234
#define MAX_TEXT 512

struct msg_st {
  long int msg_type;
  char text[MAX_TEXT];
};

int main(int argc, char *argv[]) {
  int running = 1;
  int msgid = -1;
  struct msg_st data;
  long int msgtype = 0;

  msgid = msgget((key_t)MSG_ID, 0666 | IPC_CREAT);
  if (msgid == -1) {
    fprintf(stderr, "msgget failed with error: %d\n", errno);
    exit(EXIT_FAILURE);
  }

  while (running) {
    if (msgrcv(msgid, (void *)&data, MAX_TEXT, msgtype, 0) == -1) {
      fprintf(stderr, "msgrcv failed with errno: %d\n", errno);
      exit(EXIT_FAILURE);
    }

    if (data.msg_type != 1) {
      fprintf(stderr, "Wrong message type: %ld\n", data.msg_type);
      exit(EXIT_FAILURE);
    }

    printf("You wrote: %s\n", data.text);

    if (strncmp(data.text, "end", 3) == 0)
      running = 0;
  }

  if (msgctl(msgid, IPC_RMID, 0) == -1) {
    fprintf(stderr, "msgctl(IPC_RMID) failed\n");
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
