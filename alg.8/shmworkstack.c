#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/shm.h>

#include "shmdata.h"

int main(int argc, char *argv[]){
    void *shmptr = NULL;
    struct shared_struct *shared;
    int shmid;
    key_t key;
    char operation[BUFSIZ + 1];

    sscanf(argv[1], "%x", &key);
    printf("%*sshmread: IPC key = %x\n", 30, " ", key);
    
    shmid = shmget((key_t)key, sizeof(struct shared_struct), 0666|PERM);
    if (shmid == -1) {
        ERR_EXIT("shread: shmget()");
    }

    shmptr = shmat(shmid, 0, 0);
    if(shmptr == (void *)-1) {
        ERR_EXIT("shread: shmat()");
    }
    printf("%*sshmread: shmid = %d\n", 30, " ", shmid);    
    printf("%*sshmread: shared memory attached at %p\n", 30, " ", shmptr);
    printf("%*sshmread process ready ...\n", 30, " ");
    
    shared = (struct shared_struct *)shmptr;

    while(1){
        while(shared->lock == 0){
            sleep(1);
        }
        shared->lock = 0;
        printf("输入你想进行的操作：");
        scanf("%s",operation);
        if(strncmp(operation,"push",4) == 0){
            int no_;
            char name_[NAME_SIZE];
            printf("输入学号：");
            scanf("%d",&no_);
            printf("输入姓名： ");
            scanf("%s",name_);
            Push(shared,shared->heap,no_,name_);
            shared->lock = 1;
            sleep(1);
        }
        else if(strncmp(operation,"pop",3) == 0){
            Pop(shared,shared->heap);
            shared->lock = 1;
            sleep(1);
        }
        else if(strncmp(operation,"change",6) == 0){
            Change(shared);
            shared->lock = 1;
            sleep(1);
        }
        else if(strncmp(operation,"show",4) == 0){
            Show(shared);
            shared->lock = 1;
            sleep(1);
        }
        else if(strncmp(operation,"sort",4) == 0){
            Sort(shared,shared->heap);
            shared->lock = 1;
            sleep(1);
        }
        else if(strncmp(operation,"exit",4) == 0) {
            shared->lock = 1;
            break;
        }
        else{
            shared->lock = 1;
        }
    }

    if (shmdt(shmptr) == -1) {
            ERR_EXIT("shmread: shmdt()");
    }
 
    sleep(1);
    exit(EXIT_SUCCESS);
}

