#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int fd[2];
  pid_t pid;
  char buffer[1024];
  FILE *fpin, *fpout;

  memset(buffer, 0, sizeof(buffer));
  memset(fd, 0, sizeof(fd));
  if (pipe(fd) == -1) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  if ((pid = fork()) == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  } else if (pid > 0) {
    close(fd[0]);
    write(fd[1], "Hello, World!", 13);
  } else {
    close(fd[1]);
    read(fd[0], buffer, 13);
    write(STDOUT_FILENO, buffer, 13);
  }

  memset(buffer, 0, sizeof(buffer));
  fpin = popen("ls -l", "r");
  fpout = fopen("output.txt", "w+");
  fread(buffer, sizeof(char), sizeof(buffer), fpin);
  fwrite(buffer, sizeof(char), sizeof(buffer), fpout);
  pclose(fpin);
  fclose(fpout);

  exit(EXIT_SUCCESS);
}
