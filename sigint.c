/*GRABS SIGINT*/

#include <stdio.h>
#include<unistd.h>
#include<errno.h>
#include<stdlib.h>
#include<signal.h>

void sigint_handler(int sig)
{
    (void)sig; // remove unused variable warning
    write(0, "Ahhh! SIGNINT\n", 14);
    // const char msg[]="Ahhh! SIGNINT\n";
    // write(0, msg, sizeof(msg));
}
int main (void)
{
    char s[200];
    struct sigaction sa = {
        .sa_handler = sigint_handler,
        .sa_flags = SA_RESTART,
        .sa_mask = 0,
    };
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction"); exit(1);
    }    
    printf("Enter a string:\n");
    if (fgets(s, sizeof s, stdin) == NULL)
    perror("fgets");
    else
    printf("you entered: %s\n", s);
return 0;
}