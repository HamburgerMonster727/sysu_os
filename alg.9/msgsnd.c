#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>

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

int main(int argc, char *argv[]){
    char pathname[80];
    struct stat fileattr;
    key_t key;
    struct msg_struct data;
    long int msg_type;
    char buffer[TEXT_SIZE];
    int msqid, ret, count = 0;

    if(argc < 2) {
        printf("Usage: ./a.out pathname\n");
        return EXIT_FAILURE;
    }
    strcpy(pathname, argv[1]);

    if(stat(pathname, &fileattr) == -1) {
        ret = creat(pathname, O_RDWR);
        if (ret == -1) {
            ERR_EXIT("creat()");
        }
        printf("shared file object created\n");
    }
    
    key = ftok(pathname, 0x27); /* project_id can be any nonzero integer */
    if(key < 0) {
        ERR_EXIT("ftok()");
    }
    
    msqid = msgget((key_t)key, 0666 | IPC_CREAT);
    if(msqid == -1) {
        ERR_EXIT("msgget()");
    }

    struct msqid_ds msqattr;
    ret = msgctl(msqid, IPC_STAT, &msqattr);

    printf("number of messages remainded = %ld, empty slots = %ld\n", msqattr.msg_qnum, 16384/TEXT_SIZE-msqattr.msg_qnum);
    printf("Blocking Sending ... \n");
    while(1){
        printf("Input massage: ");
        scanf("%s",buffer);    
        printf("msgsnd: %s\n",buffer);       
        data.msg_type = 1;
        strcpy(data.mtext, buffer);
        ret = msgsnd(msqid, (void *)&data, TEXT_SIZE, 0); /* 0: blocking send, waiting when msg queue is full */   
        if(ret == -1) {
            ERR_EXIT("msgsnd()");
        }
        if(strncmp(data.mtext,"end",3) == 0){
            break;
        }
        count++;
        sleep(1);
    }

    printf("number of sent messages = %d\n", count);
    system("ipcs -q");
    exit(EXIT_SUCCESS);
}
