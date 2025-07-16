#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>

int main(void)
{
    int pfds[2];
    pipe(pfds);

    if(!fork())
    {
        dup2(pfds[1], 1); /*combines close and dup automatically*/
        close(pfds[0]);
        close(pfds[1]);
        execlp("ls", "ls", NULL);
    }
    else{
        dup2(pfds[0], 0);
        close(pfds[0]);
        close(pfds[1]);
        execlp("wc", "wc", "-l", NULL);
    }
    return 0;
}