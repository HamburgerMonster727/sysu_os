#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <fcntl.h>
#define TEXT_SIZE 512
#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while(0)

int main(int argc, char *argv[]){
    pid_t childpid1, childpid2;
    char str[80];
    strcpy(str,"myshm");
    char *argv1[] = {" ", str, 0};
    childpid1 = vfork();
    if(childpid1 < 0) {
        ERR_EXIT("msgcon: 1st vfork()");
    } 
    else if(childpid1 == 0) {
        execv("./msgsnd.o", argv1);
    }
    else {
        childpid2 = vfork();
        if(childpid2 < 0) {
            ERR_EXIT("msgcon: 2nd vfork()");
        }
        else if (childpid2 == 0) {
            execv("./msgrcv.o", argv1); 
        }
        else {
            wait(&childpid1);
            wait(&childpid2);   
        }
    }
    exit(EXIT_SUCCESS);
}

