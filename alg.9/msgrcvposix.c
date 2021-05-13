#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <errno.h>
#define TEXT_SIZE 512
#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while(0)

int main(int argc, char *argv[]){ /* Usage: ./b.out pathname msg_type */
    char pathname[80];
    int ret, count = 0;
    mqd_t mqID;
    struct mq_attr mqAttr;
    char buffer[TEXT_SIZE];
    unsigned int prio;

    if(argc < 2) {
        printf("Usage: ./b.out pathname\n");
        return EXIT_FAILURE;
    }
    strcpy(pathname, argv[1]);

    mqID = mq_open(pathname,O_RDONLY,0666,NULL);
    if(mqID < 0) {
        ERR_EXIT("mq_open()");
    }	
    
    ret = mq_getattr(mqID,&mqAttr);
    if(ret == -1) {
        ERR_EXIT("mq_getattr()");
    }

    while(1){
        ret = mq_receive(mqID,buffer,mqAttr.mq_msgsize,&prio);
        if(ret == -1) { 
            ERR_EXIT("mq_receive()");
        }
        printf("%*smsgrcv:%d %s\n",30," ",prio,buffer);
        if(strncmp(buffer,"end",3) == 0){
            break;
        }
        count++;
    }
    printf("number of received messages = %d\n", count);
    mq_close(mqID);
    ret = mq_unlink(pathname);
    if(ret == -1) { 
        ERR_EXIT("mq_unlink()");
    }
    exit(EXIT_SUCCESS);
}
