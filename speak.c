/*writes into a FIFO*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<errno.h>
#include<unistd.h>
#include<fcntl.h>

#define FIFO_NAME "neotic_order"

int main(void)
{
    char s[300];
    int num, fd;
    mkfifo(FIFO_NAME, 0644);
    printf("Waiting for readers...\n");
    fd = open(FIFO_NAME, O_WRONLY);
    printf("Got a reader--type some stuff\n");
    while(fgets(s, sizeof s, stdin), !feof(stdin)){
        if((num = write(fd, s, strlen(s)))==-1){
            perror("write");
        }
        else
        {
            printf("speak: wrote %d bytes\n", num);
        }
    }
    return 0;
}