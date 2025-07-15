#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    pid_t pid;
    int rv;

    switch (pid = fork()){
    case -1:
        perror("fork");
        exit(-1);
    case 0:
        printf("CHILD: This is the child process\n");
        printf("CHILD: My pid is %d\n", getpid());
        printf("CHILD: My parent's pid is %d\n", getppid());
        printf("CHILD: Enter my exit status (make it small): ");
        scanf(" %d", &rv);
        printf("CHILD: I'm outta here!\n");
        exit(rv);
    
    default:
        printf("PARENT: This is the parent process\n");
        printf("PARENT: My pid is %d\n", getpid());
        printf("PARENT: My child's pid is %d\n", pid);
        printf("PARENT: I'm now waiting for my child to exit...\n");
        wait(&rv);
        printf("PARENT: My child's exit status %d\n", WEXITSTATUS(rv));
        printf("PARENT: I'm outta here!");

    }
    return 0;

}