#include <string.h>
#define TEXT_SIZE 4*1024  /* = PAGE_SIZE, size of each message */
#define TEXT_NUM 20     /* maximal number of mesages */
    /* total size can not exceed current shmmax,
       or an 'invalid argument' error occurs when shmget */
#define true 1
#define false 0

/* a demo structure, modified as needed */
typedef struct shared_struct {
    int front;
    int rear;
    char mtext[TEXT_NUM][TEXT_SIZE]; /* buffer for message reading and writing */
}shared_struct;

#define PERM S_IRUSR|S_IWUSR|IPC_CREAT

#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while(0)

void initQueue(shared_struct *queue_q){
    if(queue_q->mtext != NULL){//队列内存分配成功
        queue_q->front = queue_q->rear = 0; //初始化头尾指针
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
    if(is_fullQueue(queue_q) != true){//队列未满
        strncpy(queue_q->mtext[queue_q->rear],value,TEXT_SIZE);
        queue_q->rear = (queue_q->rear + 1)%TEXT_NUM ;//尾指针偏移
    }
}

char *get_Queue(shared_struct *queue_q){
    if(is_emptyQueue(queue_q) != true){//队列未空
        return queue_q->mtext[queue_q->front];
    }
    return NULL;
}

char *out_Queue(shared_struct *queue_q){
    if(is_emptyQueue(queue_q) != true){//队列未空
        int front = queue_q->front;
        queue_q->front = (queue_q->front + 1)%TEXT_NUM ;
        return queue_q->mtext[front];
    }
    return NULL;
}


