#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <errno.h>
#include <unistd.h>
#define TEXT_SIZE 512
#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while(0)

int main(int argc, char *argv[]){
    char pathname[80];
    long int prio;
    char buffer[TEXT_SIZE];
    int ret, count = 0;
    mqd_t mqID;

    if(argc < 2) {
        printf("Usage: ./a.out pathname\n");
        return EXIT_FAILURE;
    }
    strcpy(pathname, argv[1]);
    
    mqID = mq_open(pathname,O_WRONLY | O_CREAT,0666,NULL);
    if(mqID < 0) {
        ERR_EXIT("mq_open()");
    }
	
    while(1){
        printf("Input prio and message: ");
        scanf("%ld %s",&prio,buffer);
        printf("msgsnd: %ld %s\n", prio, buffer);
        ret = mq_send(mqID,buffer,TEXT_SIZE,prio);
        if(ret == -1) {
            ERR_EXIT("mq_send()");
        }
        if(strncmp(buffer,"end",3) == 0){
            break;
        }
        count++;
        sleep(1);
    }
    printf("number of sent messages = %d\n", count);
    mq_close(mqID);
    exit(EXIT_SUCCESS);
}
