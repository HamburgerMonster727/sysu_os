#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define TEXT_SIZE 1024

/* establishing two named pipes for dialog between two arbitrary processes
   starting from terminal-1 with ./a.out pathname 1
   starting from terminal-2 with ./a.out pathname 2 
*/
/* when parent/child terminated, it calls kill(, SIGKILL) to terminate its child/parent */

   
int main(int argc, char *argv[])
{
    char fifoname_1[80], fifoname_2[80];
	char write_msg[TEXT_SIZE], read_msg[TEXT_SIZE];
    int fdr, fdw, ret;
    pid_t pid;

    if(argc < 3) {
        printf("Usage: ./a.out pathname 1|2\n");
        return EXIT_FAILURE;
    }

    strcpy(fifoname_1, argv[1]); 
    strcpy(fifoname_2, argv[1]); 
    strcat(fifoname_1,"-1");
    strcat(fifoname_2,"-2");

    if(access(fifoname_1, F_OK) == -1) {
        if((mkfifo(fifoname_1, 0666)) != 0) {
            perror("mkfifo()");
            exit(EXIT_FAILURE);
        }
        else {
            printf("new fifo %s created ...\n", fifoname_1);
        }
    }
    if(access(fifoname_2, F_OK) == -1) {
        if((mkfifo(fifoname_2, 0666)) != 0) {
            perror("mkfifo()");
            exit(EXIT_FAILURE);
        }
        else {
            printf("new fifo %s created ...\n", fifoname_2);
        }
    }

    printf("\n==== pipe write end ====           ==== pipe read end ====\n");

    pid = fork();
    if(pid < 0) {
        perror("fork()");
        exit(EXIT_SUCCESS);
    }
        
    if(pid == 0) { /* child pro */
        if(argv[2][0] == '1') {
            fdr = open(fifoname_1, O_RDONLY);
        } else {
            fdr = open(fifoname_2, O_RDONLY);
        }
        if(fdr < 0) {
            perror("fdr open()");
        } else {
            while (1) {
                ret = read(fdr, read_msg, TEXT_SIZE); /* blocking read */
                if(ret <= 0) /* if write-end error */
                    break;
                printf("\n%*.s%s", 40, " ", read_msg);
                if(strncmp(read_msg, "end", 3) == 0) 
                    break;
            }
            kill(getppid(), SIGKILL);
            exit(0);
        }
    } else { /* parent pro */
        if(argv[2][0] == '1') {
            fdw = open(fifoname_2, O_RDWR);
        } else {
            fdw = open(fifoname_1, O_RDWR);
        }
        if(fdw < 0) {
            perror("fdw open()");
        } else {
            while (1) {
                printf("\n");
                fgets(write_msg, TEXT_SIZE, stdin);
                ret = write(fdw, write_msg, TEXT_SIZE); /* non-blocking write */ 
                if(ret <= 0)
                    break;
                if(strncmp(write_msg, "end", 3) == 0) 
                    break;
            }    
            kill(pid, SIGKILL);
        }
    }
    
    wait(0);
    close(fdr);
    close(fdw);

    exit(EXIT_SUCCESS);
}
