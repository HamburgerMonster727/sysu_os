#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <fcntl.h>

#include "shmdata.h"

int main(int argc, char *argv[]){
    struct stat fileattr;
    key_t key; /* of type int */
    int shmid; /* shared memory ID */
    void *shmptr;
    struct shared_struct *shared; /* structured shm */
    char pathname[80], key_str[10], cmd_str[80];
    int shmsize, ret;

    shmsize = sizeof(struct shared_struct);
    if(argc <2) {
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

    key = ftok(pathname, 0x27); 
    if(key == -1) {
        ERR_EXIT("shmcon: ftok()");
    }
    printf("key generated: IPC key = %x\n", key);

    shmid = shmget((key_t)key, shmsize, 0666|PERM);
    if(shmid == -1) {
        ERR_EXIT("shmcon: shmget()");
    }
    printf("shmcon: shmid = %d\n", shmid);

    shmptr = shmat(shmid, 0, 0); 
    printf("shmcon: shared Memory attached at %p\n", shmptr);

    shared = (struct shared_struct *)shmptr;
    shared->heap_num = 0;
    shared->lock = 1;

    int number;
    int no_;
    char name_[NAME_SIZE];
    printf("输入你想初始化的个数：");
    scanf("%d",&number);
    for(int i = 1;i <= number;i++){
        printf("输入学号：");
        scanf("%d",&no_);
        printf("输入姓名： ");
        scanf("%s",name_);
        Push(shared,shared->heap,no_,name_);
    }

    sprintf(key_str, "%x", key);
    sprintf(cmd_str,"gnome-terminal -x ./shmwork '%s'\n",key_str);
    system(cmd_str);
    system(cmd_str);

    exit(EXIT_SUCCESS);
}