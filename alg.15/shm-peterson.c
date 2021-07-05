#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <pthread.h>

#define true 1
#define false 0
#define MAX_N 1024         //最大线程数
#define TEXT_SIZE 4*1024   //PAGE_SIZE, size of each message 
#define TEXT_NUM 10        //循环队列最大数量
#define PERM S_IRUSR|S_IWUSR|IPC_CREAT
#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while(0)

static int counter = 0;
int level[MAX_N];     //线程等级 
int waiting[MAX_N-1]; //线程等待
int num[MAX_N-1];     //用于解决创造线程冲突
int max_num = 20;     //当前最大线程数
key_t key;            //共享内存空间key
int text = 1;         //用于写信息

typedef struct shared_struct{          //共享内存空间为循环队列结构
    int front;                         //队列头
    int rear;                          //队列尾
    char message[TEXT_NUM][TEXT_SIZE];   //消息队列
}shared_struct;

void InitQueue(shared_struct *queue_q){
    if(queue_q->message != NULL){
        queue_q->front = queue_q->rear = 0; 
    }
}

unsigned char is_fullQueue(shared_struct *queue_q){
    if((queue_q->rear +1)%TEXT_NUM == queue_q->front){
        return true;
    }
	else
        return false;
}

unsigned char is_emptyQueue(shared_struct *queue_q){
    if(queue_q->front == queue_q->rear){
        return true;
    }
    else
        return false;
}

void In_Queue(shared_struct *queue_q,char *value){
    if(is_fullQueue(queue_q) != true){
        strncpy(queue_q->message[queue_q->rear],value,TEXT_SIZE);
        queue_q->rear = (queue_q->rear + 1)%TEXT_NUM ;
    }
}

char *Out_Queue(shared_struct *queue_q){
    if(is_emptyQueue(queue_q) != true){
        int front = queue_q->front;
        queue_q->front = (queue_q->front + 1)%TEXT_NUM ;
        return queue_q->message[front];
    }
    return NULL;
}

//对共享内存空间进行写操作
static void *shmwrite(void *arg){
    int *s_addr = (int *)arg;
    int thread_num = *s_addr;
    void *shmptr = NULL;
    struct shared_struct *shared = NULL;
    int shmid;
    char buffer[BUFSIZ + 1]; 

    //连接上共享内存空间
    shmid = shmget((key_t)key, TEXT_NUM*sizeof(struct shared_struct), 0666|PERM);
    if (shmid == -1) {
        ERR_EXIT("shmwrite: shmget()");
    }
    shmptr = shmat(shmid, 0, 0);
    if(shmptr == (void *)-1) {
        ERR_EXIT("shmwrite: shmat()");
    }
    shared = (struct shared_struct *)shmptr;

    //peterson实现线程互斥
    int lev, k;
    while(1){ 
        for(lev = 0;lev < max_num-1;lev++){
            level[thread_num] = lev;
            waiting[lev] = thread_num;
            //该线程的waiting被其他线程所占领，可以升级
            while(waiting[lev] == thread_num){
                for(k = 0;k < max_num;k++){
                    //当没有线程等级比当前线程高时，可以升级
                    if(level[k] >= lev && k != thread_num){
                        break;
                    }
                    if(waiting[lev] != thread_num){
                        break;
                    }
                }
                if(k == max_num ){
                    break;
                }
            }
        }
        //进入cs，开始对内存进行写操作
        if(is_fullQueue(shared) == false){
            sprintf(buffer,"%d",text);
            printf("thread%d write: %d\n",thread_num,text);
            In_Queue(shared,buffer);
            text++;
        }
        counter++;
        if(counter > 1){
            kill(getpid(),SIGKILL);
        }
        counter--;
        //退出ls
        level[thread_num] = -1;
    }
    pthread_exit(0);
}

//对共享内存空间进行读操作
static void *shmread(void *arg){
    int *s_addr = (int *)arg;
    int thread_num = *s_addr;
    void *shmptr = NULL;
    struct shared_struct *shared;
    int shmid;

    //连接上共享内存空间
    shmid = shmget((key_t)key, TEXT_NUM*sizeof(struct shared_struct), 0666|PERM);
    if (shmid == -1) {
        ERR_EXIT("shmread: shmget()");
    }
    shmptr = shmat(shmid, 0, 0);
    if(shmptr == (void *)-1) {
        ERR_EXIT("shmread: shmat()");
    }
    shared = (struct shared_struct *)shmptr;

    //peterson实现线程互斥
    int lev, k;
    while(1){
         for(lev = 0;lev < max_num-1;lev++){
            level[thread_num] = lev;
            waiting[lev] = thread_num;
            while(waiting[lev] == thread_num){
                for(k = 0;k < max_num;k++){
                    if(level[k] >= lev && k != thread_num){
                        break;
                    }
                    if(waiting[lev] != thread_num){
                        break;
                    }
                }
                if(k == max_num){
                    break;
                }
            }
        }
        //进入cs，开始对内存进行读操作
        if(is_emptyQueue(shared) == false){
            printf("%*sthread%d read: %s\n", 30, " ", thread_num, Out_Queue(shared));
        }
        counter++;
        if(counter > 1){
            kill(getpid(),SIGKILL);
        }
        counter--;
        //退出ls
        level[thread_num] = -1;
    }
    pthread_exit(0);
}
 
int main(int argc, char *argv[]){
    struct stat fileattr;
    int shmid; 
    void *shmptr;
    struct shared_struct *shared;
    char pathname[80];
    int shmsize, ret;

    //创建共享内存空间
    shmsize = TEXT_NUM*sizeof(struct shared_struct);
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
    shmid = shmget((key_t)key, shmsize, 0666|PERM);
    if(shmid == -1) {
        ERR_EXIT("shmcon: shmget()");
    }
    shmptr = shmat(shmid, 0, 0); 
    if(shmptr == (void *)-1) {
        ERR_EXIT("shmcon: shmat()");
    }
    shared = (struct shared_struct *)shmptr;
    InitQueue(shared);
    if(shmdt(shmptr) == -1) {
        ERR_EXIT("shmcon: shmdt()");
    }

    //输入要创建的写线程和读线程的数量
    int write_num,read_num;
    printf("input the number of write:\n");
    scanf("%d",&write_num);
    printf("input the number of read:\n");
    scanf("%d",&read_num);
    max_num = read_num + write_num;
    
    memset(level, (-1), sizeof(level));
    memset(waiting, (-1), sizeof(waiting));

    //创建读写线程
    int i;
    for (i = 0; i < max_num; i++) {
        num[i] = i;
    }
    pthread_t ptid[max_num];
    for (i = 0; i < max_num; i++) {
        if(i < write_num){//先创建写线程
            ret = pthread_create(&ptid[i], NULL, &shmwrite, (void *)&num[i]);
        }
        else{//再创建读线程
            ret = pthread_create(&ptid[i], NULL, &shmread, (void *)&num[i]);
        }
        if(ret != 0) {
            fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
            exit(1);
        }
    }
    for (int i = 0; i < max_num; i++) {
        ret = pthread_join(ptid[i], NULL);
        if(ret != 0) {
            fprintf(stderr, "pthread_join error: %s\n", strerror(ret));
            exit(1);
        }
    }

    return 0;
}