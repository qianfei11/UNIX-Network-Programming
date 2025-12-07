#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>

#define PATHNAME "."
#define PROJ_ID 0X6666

int creatsem(int nums);
int initsem(int semid, int nums, int initval);
int getsem(int nums);
int p(int semid, int who);
int v(int semid, int who);
int destroysem(int semid);

union semun {
  int val;
};

int commsem(int nums, int flags) {
  key_t key;
  int semid;

  key = ftok(PATHNAME, PROJ_ID);
  if (key < 0) {
    perror("ftok");
    return -1;
  }

  semid = semget(key, nums, flags);
  if (semid < 0) {
    perror("semget");
    return -2;
  }

  return semid;
}

int creatsem(int nums) {
  int ret;

  ret = commsem(nums, IPC_CREAT | IPC_EXCL | 0666);

  return ret;
}

int initsem(int semid, int nums, int value) {
  union semun u;
  u.val = value;
  if (semctl(semid, nums, SETVAL, u) < 0) {
    perror("semctl");
    return -1;
  }
  return 0;
}

int getsem(int nums) {
  int ret;

  ret = commsem(nums, IPC_CREAT);

  return ret;
}

int commpv(int semid, int who, int op) {
  struct sembuf sf;
  sf.sem_num = who;
  sf.sem_op = op;
  sf.sem_flg = 0;
  if (semop(semid, &sf, 1) < 0) {
    perror("semop");
    return -1;
  }
  return 0;
}

int p(int semid, int who) {
  int ret;

  ret = commpv(semid, who, -1);

  return ret;
}

int v(int semid, int who) {
  int ret;

  ret = commpv(semid, who, 1);

  return ret;
}

int destroysem(int semid) {
  if (semctl(semid, 0, IPC_RMID) < 0) {
    perror("semctl");
    return -1;
  }

  return 0;
}

int main(int argc, char *argv[]) {
  int semid, _semid;
  pid_t pid;

  semid = creatsem(1);
  initsem(semid, 0, 1);
  pid = fork();
  _semid = getsem(0);
  if (pid == 0) {
    while (1) {
      p(_semid, 0);
      printf("A");
      fflush(stdout);
      usleep(100000);
      printf("A ");
      fflush(stdout);
      usleep(300000);
      v(_semid, 0);
    }
  } else {
    while (1) {
      p(_semid, 0);
      printf("B");
      fflush(stdout);
      usleep(200000);
      printf("B ");
      fflush(stdout);
      usleep(100000);
      v(_semid, 0);
    }
  }
  destroysem(semid);

  return 0;
}
