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
  struct msg_st data;
  char buffer[BUFSIZ];
  int msgid = -1;

  msgid = msgget((key_t)MSG_ID, 0666 | IPC_CREAT);
  if (msgid == -1) {
    fprintf(stderr, "msgget failed with error: %d\n", errno);
    exit(EXIT_FAILURE);
  }

  while (running) {
    printf("Enter some text: ");
    fgets(buffer, BUFSIZ, stdin);
    data.msg_type = 1;
    strcpy(data.text, buffer);

    if (msgsnd(msgid, (void *)&data, MAX_TEXT, 0) == -1) {
      fprintf(stderr, "msgsnd failed\n");
      exit(EXIT_FAILURE);
    }

    if (strncmp(buffer, "end", 3) == 0)
      running = 0;
    sleep(1);
  }

  exit(EXIT_SUCCESS);
}
