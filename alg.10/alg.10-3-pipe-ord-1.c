#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define READ_END 0
#define WRITE_END 1

/* a blocking read version */

int main(void)
{
    char write_msg[BUFSIZ]; /* BUFSIZ = 8192bytes, saved from stdin */
    char read_msg[BUFSIZ];
    int pipefd[2]; /* pipefd[0] for READ_END, pipefd[1] for WRITE_END */
    int flags;
    pid_t pid;
            
    if(pipe(pipefd) == -1) { /* create a pipe */
        perror("pipe()");
        exit(EXIT_FAILURE);
    }
    flags = fcntl(pipefd[WRITE_END], F_GETFL);
    fcntl(pipefd[WRITE_END], F_SETFL, flags | O_NONBLOCK); /* non-blocking write */
    flags = fcntl(pipefd[READ_END], F_GETFL);
    fcntl(pipefd[READ_END], F_SETFL, flags); /* blocking read */
    
    pid = fork(); /* fork a child process */
    if(pid < 0) {
        perror("fork()");
        exit(EXIT_FAILURE);
    }
    if(pid > 0) { /* parent process */
        while (1) {
	    printf("Enter some text: ");
            fgets(write_msg, BUFSIZ, stdin);
            write(pipefd[WRITE_END], write_msg, BUFSIZ);
            if(strncmp(write_msg, "end", 3) == 0) { 
                break;
            }
            sleep(1);
        }
    }
    else { /* child process */
        while (1) {
            read(pipefd[READ_END], read_msg, BUFSIZ);
            printf("\n\t\t\t\t\tpipe read = %s", read_msg);
            if(strncmp(read_msg, "end", 3) == 0) {
                break;
            }
        }
    }
    
    wait(0);
    close(pipefd[WRITE_END]);
    close(pipefd[READ_END]);

    exit(EXIT_SUCCESS);
}
