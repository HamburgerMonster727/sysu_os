#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define TEXT_SIZE 1024

/* write only & non-blocking write version */
int main(int argc, char *argv[])
{
    char fifoname[80], write_msg[TEXT_SIZE];
    int fdw;

    if(argc < 2) {
        printf("Usage: ./a.out pathname\n");
        return EXIT_FAILURE;
    }
    strcpy(fifoname, argv[1]);
    if(access(fifoname, F_OK) == -1) {
        if(mkfifo(fifoname, 0666) != 0) {
            perror("mkfifo()");
            exit(EXIT_FAILURE);
        }
        else {
            printf("new fifo %s named pipe created\n", fifoname);
        }
    }

    int count = 10;
    while (count) {
        fdw = open(fifoname, O_WRONLY | O_NONBLOCK);
            /* non-blocking send, return error if the receiving end not ready */
        if(fdw < 0) {
            printf("waiting for receiver ... %d\n", count);
            sleep(1); 
                /* do something, and query again, or exit(EXIT_FAILURE) when time out  */
            count--;
        }
        else {
            break;
        }
    }

    while (count) {
        printf("\nEnter some text: ");
        fgets(write_msg, TEXT_SIZE, stdin);
        write(fdw, write_msg, TEXT_SIZE); /* non-blocking write */  
        if (strncmp(write_msg, "end", 3) == 0) {
            break;
        }
        sleep(1);
    }
	
    close(fdw);

    exit(EXIT_SUCCESS);
}
