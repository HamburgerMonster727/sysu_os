#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define TEXT_SIZE 1024

/* establishing two named pipes for dialog between two arbitrary processes on two terminals
   starting from terminal-1 with ./a.out pathname 1
   starting from terminal-2 with ./a.out pathname 2
*/
/* two ordinary pipes are used to build a bidirectional connection between child (read pro) 
   and parent (write pro); any terminated any pro will cause the other pro terminated */

/* alternatively, kill(, SIGKILL) can be used with less overheads, but less reasonability */

   
int main(int argc, char *argv[])
{
    char fifoname_1[80], fifoname_2[80];
    char write_msg[TEXT_SIZE], read_msg[TEXT_SIZE];
    int fdr, fdw, ret;
    pid_t pid;

    int pipefd1[2], pipefd2[2], flags; char msg_str[2];

    if(argc < 3) {
        printf("Usage: ./a.out pathname 1|2\n");
        return EXIT_FAILURE;
    }

    if(pipe(pipefd1) == -1) {
        perror("pipe()");
        exit(EXIT_FAILURE);
    }

    flags = fcntl(pipefd1[1], F_GETFL);
    fcntl(pipefd1[1], F_SETFL, flags | O_NONBLOCK);
    flags = fcntl(pipefd1[0], F_GETFL);
    fcntl(pipefd1[0], F_SETFL, flags | O_NONBLOCK);

    if(pipe(pipefd2) == -1) {
        perror("pipe()");
        exit(EXIT_FAILURE);
    }

    flags = fcntl(pipefd2[1], F_GETFL);
    fcntl(pipefd2[1], F_SETFL, flags | O_NONBLOCK);
    flags = fcntl(pipefd2[0], F_GETFL);
    fcntl(pipefd2[0], F_SETFL, flags | O_NONBLOCK);

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
            fdr = open(fifoname_1, O_RDONLY | O_NONBLOCK);
        } else {
            fdr = open(fifoname_2, O_RDONLY | O_NONBLOCK);
        }
        if(fdr < 0) {
            perror("fdr open()");
        } else {
            while (1) {
                ret = read(fdr, read_msg, TEXT_SIZE); /* non-blocking read, cannot sense write-end error */
                if(ret > 0) {
                    printf("\n%*.s%s", 40, " ", read_msg);
                    if(strncmp(read_msg, "end", 3) == 0) 
                        break;
                }
                ret = read(pipefd2[0], msg_str, 2);
                if(ret > 0 && msg_str[0] == '1') { /* parent pro terminated */
                    break;
                }
            }
        }
        write(pipefd1[1], "1", 2); /* inform parent to terminate  */
        //kill(getppid(), SIGKILL);
        exit(0);
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
                ret = read(pipefd1[0], msg_str, 2);
                if(ret > 0 && msg_str[0] == '1') /* child pro terminated */
                    break;
            }
        } 
        write(pipefd2[1], "1", 2); /* inform child pro to terminat */
        //kill(pid, SIGKILL);
    }
    
    wait(0);
    close(fdr);
    close(fdw);
    close(pipefd1[1]);
    close(pipefd1[0]);
    close(pipefd2[1]);
    close(pipefd2[0]);

    exit(EXIT_SUCCESS);
}
