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
    char buffer[TEXT_SZ];
    int shmid;

    shmid = shmget((key_t)1, SHARE_BUF, 0666 | IPC_CREAT);
    if (shmid == -1)
    {
        fprintf(stderr, "Shmget failed!\n");
        exit(-1);
    }

    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1)
    {
        fprintf(stderr, "Shmat failed!\n");
        exit(-1);
    }

    shared_stuff = (struct shared_use_st *)shared_memory;
    while (running)
    {
        while (shared_stuff->written_by_you == 1)
        {
            sleep(1);
        }
        printf("Enter some text: ");
        fgets(buffer, TEXT_SZ, stdin);

        strncpy(shared_stuff->some_text, buffer, TEXT_SZ);
        shared_stuff->written_by_you = 1;

        if (strncmp(buffer, "end", 3) == 0)
        {
            running = 0;
        }
    }

    if (shmdt(shared_memory) == -1)
    {
        fprintf(stderr, "Shmdt failed\n");
        exit(-1);
    }

    exit(0);
}
