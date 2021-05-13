#include <stdio.h>  
#include <stdlib.h>
#include<unistd.h>  
#include <fcntl.h>

#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while(0)

int main(int argc, char *argv[])
{
    int pipefd[2];
    int bufsize;
    char *buffer;
    
    if(pipe(pipefd) == -1) /* blocking write and blocking read in default */
        /* blocking write: blocked when buffer is full
           non-blocking write: return -1 when buffer is full
           blocking read: blocked when buffer is empty
           non-blocking read: return -1 when buffer is empty
        */
        ERR_EXIT("pipe()");

    int flags, ret, lastwritten, count, totalwritten;

    flags = fcntl(pipefd[1], F_GETFL);
    fcntl(pipefd[1], F_SETFL, flags | O_NONBLOCK); /* set write_end NONBLOCK */
   
    if(argc < 2) {
        printf("Example: ./a.out 1024\n");
        exit(EXIT_FAILURE);
    }
    
    bufsize = atoi(argv[1]);
    printf("testing buffer size = %d\n", bufsize);
    buffer = (char *)malloc(bufsize*sizeof(char));
    if(buffer == NULL || bufsize == 0) {
        ERR_EXIT("malloc()");
    }
 
    count = 0;
    while (1) {
        ret = write(pipefd[1], buffer, bufsize);
            /* bufsize is better to be 2^k */
        if(ret == -1) {
            perror("write()");
            break;
        }
        lastwritten = ret;
        printf("count = %d\n", count);
        count++;
    }
    totalwritten = (count-1)*bufsize + lastwritten;
    printf("single pipe buffer count =  %d, last written = %d bytes\n", count, lastwritten);
    printf("total written = %d bytes = %d KiB\n", totalwritten, totalwritten/1024); /* pipe buffer */

    return 0;
}
