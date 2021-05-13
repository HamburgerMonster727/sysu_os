#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define TEXT_SIZE 1024

/* blocking read version */
int main(int argc, char *argv[])
{
    char fifoname[80], read_msg[TEXT_SIZE];
    int fdr;
    
    if(argc < 2) {
        printf("Usage: ./a.out pathname\n");
        return EXIT_FAILURE;
    }
    strcpy(fifoname, argv[1]);
    if(access(fifoname, F_OK) == -1) {
        if (mkfifo(fifoname, 0666) != 0) {
            perror("mkfifo()");
            exit(EXIT_FAILURE);
        }
        else {
            printf("new fifo %s named pipe created\n", fifoname);
        }
    }
    
    fdr = open(fifoname, O_RDONLY);  /* blocking read */
    if (fdr < 0) {
        perror("pipe read open()");
        exit(EXIT_FAILURE);
    }
    else {
        while (1) {
            read(fdr, read_msg, TEXT_SIZE);
            printf("\npipe read_end = %s",read_msg);
            if (strncmp(read_msg, "end", 3) == 0) { /* keep working until "end" received */
              	break;   
            }
        }
    }

    close(fdr);

    exit(EXIT_SUCCESS);
}
