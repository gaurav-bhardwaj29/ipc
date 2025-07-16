/*reads data from FIFO*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

#define FIFO_NAME "neotic_order"

int main(void)
{
    char s[300];
    int fd, num;
    mkfifo(FIFO_NAME, 0644);
    printf("Waiting for writers...\n");
    fd = open(FIFO_NAME, O_RDONLY);
    printf("got a writer\n");

    do{
        if((num = read ( fd, s, sizeof(s)))==-1)
        perror("read");
    else
    {
        s[num]='\0';
        printf("tick: read %d bytes: \"%s\"\n", num, s);

    }
    } while (num>0);
return 0;
}
