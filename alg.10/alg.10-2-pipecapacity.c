#include <stdio.h>  
#include <stdlib.h>
#include <unistd.h>  
#include <fcntl.h>

#define  PIPE_SIZE 64*1024  /* 64 KiB */

int main(void)
{
    char buf[PIPE_SIZE];
    int testfd[600][2];
    int i;
    long int ret;
    
    for (i = 0; i < 600; i++) {
        if(pipe(testfd[i]) == -1) {
            perror("pipe()");
            break;
        } 
        fcntl(testfd[i][1], F_SETFL, O_NONBLOCK); 
        ret = write(testfd[i][1], buf, PIPE_SIZE);
        if(ret == -1 || ret != PIPE_SIZE) {
            perror("write()");
            break;
        }
//        printf("writing bytes = %ld\n", ret);
    }
    
    printf("\nsingle pipe buffer = 64 KiB, pipes created: %d\n", i);
    printf("total used size: %ld bytes = %ld KiB, or %.0f MiB\n", (long int)i*PIPE_SIZE, (long int)i*PIPE_SIZE/1024, (double)i*PIPE_SIZE/1024/1024);

    return 0;
}

