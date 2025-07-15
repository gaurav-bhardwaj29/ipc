/*GRABS SIGINT*/

#include <stdio.h>
#include<unistd.h>
#include<errno.h>
#include<stdlib.h>
#include<signal.h>

volatile sig_atomic_t got_usr1;

void sigint_handler(int sig)
{
    got_usr1=1;
}
int main (void)
{
    char s[200];
    struct sigaction sa = {
        .sa_handler = sigint_handler,
        .sa_flags = SA_RESTART,
        .sa_mask = 0,
    };
    got_usr1=0;
    if (sigaction(SIGUSR1, &sa, NULL) == -1)
    {
        perror("sigaction"); exit(1);
    }    
    while(!got_usr1)
    {
        printf("PID %d: working too hard...\n", getpid());
        sleep(1);
    }
    printf("Done in by SIGUSR1\n");
return 0;
}