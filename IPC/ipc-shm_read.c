#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>

#define SHARE_BUF 16 * 1024
#define TEXT_SZ 2048

struct shared_use_st
{
    int written_by_you;
    char some_text[TEXT_SZ];
};

int main()
{
    int running = 1;
    void *shared_memory = (void *)0;
    struct shared_use_st *shared_stuff;
    int shmid;

    shmid = shmget((key_t)1, SHARE_BUF, 0666 | IPC_CREAT);
    if (shmid == -1)
    {
        fprintf(stderr, "shmget failed\n");
        exit(-1);
    }

    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1)
    {
        fprintf(stderr, "shmat failed\n");
        exit(-1);
    }

    shared_stuff = (struct shared_use_st *)shared_memory;
    shared_stuff->written_by_you = 0;
    while (running)
    {
        if (shared_stuff->written_by_you)
        {
            printf("You wrote: %s", shared_stuff->some_text);
            sleep(1);
            shared_stuff->written_by_you = 0;
            if (strncmp(shared_stuff->some_text, "end", 3) == 0)
            {
                running = 0;
            }
        }
    }

    if (shmdt(shared_memory) == -1)
    {
        fprintf(stderr, "shmdt failed!\n");
        exit(-1);
    }

    if (shmctl(shmid, IPC_RMID, 0) == -1)
    {
        fprintf(stderr, "shmctl(IPC_RMID) failed!\n");
        exit(-1);
    }

    exit(0);
}

