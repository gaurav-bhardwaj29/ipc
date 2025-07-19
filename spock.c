/*reads from a message queue*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct ms_msgbuf {
    long mtype;
    char mtext[200];
}

int main(void){
    struct my_msg_buf;
    int msqid;
    key_t key ; 
    if ((key = ftok("kirk.c", 'B')) == -1)
    {
        perror("ftok");
        exit(1);
    }
    if ((msqid = msgget(key, 0644)) == -1) { /* connect to the queue */
		perror("msgget");
		exit(1);
	}
    printf("spock: ready to receive message, captain. \n")
    for (;;)
    {
        if (msgrcv(msqid, &buf, sizeof buf.mtext, 0, 0)== -1)
        {
            perror("msgrcv");
            exit(1);
        }
        printf("spock: \"%s\"\n", buf.mtext);
	}

	return 0;
    
}