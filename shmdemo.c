/*read and write to a shared memory segment*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE 1024

int main (int argc, char *argv[])
{
    key_t key;
    int shmid;
    int *data;
    if (argc > 2)
    {
        fprintf(stderr, "usage: shmdemo [data_to_write]\n");
        exit(1);
    }
    if ((key = ftok("shmdemo.c", 'R')) == -1)
    {
        perror("ftok");
        exit(1);
    }
    if ((shmid=shmget(key, SHM_SIZE, 0664 | IPC_CREAT)) == -1)
    {
        perror("shmget");
        exit(1);
    }
    data = shmat(shmid, (void *)0, 0);
    if (data == (void *)(-1))
    {
        perror("shmat");
        exit(1);
    }
    // read and modify the segment, based on the command line;
    if (argc == 2)
    {
        printf("writing to segment: \"%s\"\n", argv[1]);
        strncpy(data, argv[1], SHM_SIZE);
        data[SHM_SIZE-1] = '\0';
    }
    else
    printf("segment contains: \"%s\"\n", data);
    if (shmdt(data)==-1)
    {
        perror("shmdt");
        exit(1);
    }
    return 0;
}