/*writes to a message queue*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>

struct my_msgbuf {
    long mtype;
    char mtext[200];
};

int main(void)
{
    struct my_msgbuf buf;
    int msqid;
    key_t key;
    if ((key=ftok("kirk.c", 'B')) == -1)
    {
        perror("ftok");
        exit(1);
    }
    if ((msqid = msgget(key, 0644 | IPC_CREAT)) == -1)
    {
        perror ("msgget");
        exit (1);
    }
    printf("Enter lines of text, ^D to quit:\n");
    buf.mtype = 1;
    while(fgets(buf.mtext, sizeof buf.mtext, stdin) != NULL){
        int len = strlen (buf.mtext);
        if (buf.mtext[len-1]=='\n') buf.mtext[len-1] = '\0';
        if (msgsnd(msqid, &buf, len, 0) == -1)
        perror("msgsnd");
    }
    if (msgctl(msqid, IPC_RMID, NULL) == -1)
    {
        perror ("msgctl");
        exit (1);
    }
    return 0;

}