#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/stat.h>

#define PERM S_IRUSR|S_IWUSR|IPC_CREAT
#define TEXT_SIZE 512
#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while(0)

struct msg_struct {
    long int msg_type;
    char mtext[TEXT_SIZE]; /* binary data */
};

int main(int argc, char *argv[]){ /* Usage: ./b.out pathname msg_type */
    key_t key;
    struct stat fileattr;
    char pathname[80];
    int msqid, ret, count = 0;
    struct msg_struct data;
    long int msgtype = 0;   /* 0 - type of any messages */

    if(argc < 2) {
        printf("Usage: ./b.out pathname msg_type\n");
        return EXIT_FAILURE;
    }
    strcpy(pathname, argv[1]);
    if(stat(pathname, &fileattr) == -1) {
        ERR_EXIT("shared file object stat error");
    }
    if((key = ftok(pathname, 0x27)) < 0) {
        ERR_EXIT("ftok()");
    }

    msqid = msgget((key_t)key, 0666); /* do not create a new msg queue */
    if(msqid == -1) {
        ERR_EXIT("msgget()");
    }

    msgtype = 1;
    while(1){
        ret = msgrcv(msqid, (void *)&data, TEXT_SIZE, msgtype, 0); //blocking
        printf("%*smsgrcv: %s\n",30," ",data.mtext);
        if(strncmp(data.mtext,"end",3) == 0){
            printf("number of received messages = %d\n", count);
            break;
        }
        count++;
    }
    
    struct msqid_ds msqattr;
    ret = msgctl(msqid, IPC_STAT, &msqattr);
    printf("number of messages remainding = %ld\n", msqattr.msg_qnum); 
    if(msqattr.msg_qnum == 0) {
        printf("do you want to delete this msg queue?(y/n)");
        if(getchar() == 'y') {
            if(msgctl(msqid, IPC_RMID, 0) == -1)
                perror("msgctl(IPC_RMID)");
        }
    }

    system("ipcs -q");
    exit(EXIT_SUCCESS);
}
