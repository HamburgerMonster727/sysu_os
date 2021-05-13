#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define TEXT_SIZE 1024

int main(int argc, char *argv[])
{
    char write_msg[TEXT_SIZE];
    char read_msg[TEXT_SIZE];
    char fifoname[80];
    int fdw, fdr;
    pid_t pid;
    
    if(argc < 2) {
        printf("Usage: ./a.out pathname\n");
        return EXIT_FAILURE;
    }

        /* pathname can not in current directory */
    strcpy(fifoname, argv[1]);
    if(access(fifoname, F_OK) == -1) { 
        if(mkfifo(fifoname, 0666) != 0) { /* creat a named pipe */
            perror("mkfifo()");
            exit(EXIT_FAILURE);
        }
        else {
            printf("new fifo %s created ...\n", fifoname);
        }
    }

    pid = fork(); /* fork a child process */
    if(pid < 0) {
        perror("fork()");
        exit(EXIT_FAILURE);
    }
    if(pid > 0) { /* parent process */
        fdw = open(fifoname, O_WRONLY); /* blocking write */
        if(fdw < 0) {
            perror("pipe write open()");
        } else {
	    while (1) {
                printf("Enter some text: ");
                fgets(write_msg, TEXT_SIZE, stdin);
                write(fdw, write_msg, TEXT_SIZE);
                if(strncmp(write_msg, "end", 3) == 0) {
                    break;
                }
                sleep(1);
            }
        }	
    }
    else { /* child process */
        fdr = open(fifoname, O_RDONLY);  /* blocking read */
        if(fdr < 0) {
            perror("pipe read open()");
        } else {
	    while (1) {
	        read(fdr, read_msg, TEXT_SIZE);
            	printf("\n\t\t\t\t\tpipe read_end = %s", read_msg);
            	if(strncmp(read_msg, "end", 3) == 0) {
                    break;
                }
	    }
	}
    }
    
    wait(0);
    close(fdw);
    close(fdr);
    unlink(fifoname);

    exit(EXIT_SUCCESS);
}
