#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#define max_thread_num 20
#define ready 0
#define run 1
#define block -1

//线程结点结构
typedef struct thread_node{
    pthread_cond_t pcond;       //状态锁
    pthread_mutex_t mutex;      //互斥锁
    int arrive_time;            //到达时间
    int serve_time;             //运行时间
    int priority;               //优先级
    int state;                  //状态
}thread_node;

//线程结点队列，用于进程调度后的线程顺序存储
typedef struct thread_queue{
    int thread_num[max_thread_num];
    int front;                        
    int rear;
}thread_queue;

thread_node node[max_thread_num];
int start_time = 0;             //用于记录每个线程开始运行时间
int next_start_time = 0;

void InitQueue(thread_queue *queue_q){
    if(queue_q->thread_num != NULL){
        queue_q->front = queue_q->rear = 0; 
    }
}

void In_Queue(thread_queue *queue_q,int num){
    if((queue_q->rear +1)%max_thread_num != queue_q->front){
        queue_q->thread_num[queue_q->rear] = num;
        queue_q->rear = (queue_q->rear + 1)%max_thread_num ;
    }
}

int Out_Queue(thread_queue *queue_q){
    if(queue_q->front != queue_q->rear){
        int front = queue_q->front;
        queue_q->front = (queue_q->front + 1)%max_thread_num ;
        return queue_q->thread_num[front];
    }
    return -1;
}

//计算线程平均等待时间
double Average_waiting_time(int sort[],int num){
    int end_time = node[sort[0]].arrive_time + node[sort[0]].serve_time;
    double waiting_time = 0;
    for(int i = 1;i < num;i++){
        if(node[sort[i]].arrive_time < end_time){
            waiting_time += (end_time - node[sort[i]].arrive_time);
            end_time += node[sort[i]].serve_time;
        }
        else{
            waiting_time += 0;
            end_time = node[sort[i]].arrive_time + node[sort[i]].serve_time;
        }
    }
    waiting_time = waiting_time / num;
    return waiting_time;
}

double FCFS(thread_queue *queue,int num){
    InitQueue(queue);

    //记录调整后的结点顺序
    int sort[max_thread_num];
    for(int i = 0;i < num;i++){
        sort[i] = i;
    }

    //按照线程到达时间从小到大排序
    for(int i = 0;i < num-1;i++){
        int flag = 1;
        for(int j = 0;j < num-1-i;j++){
            if(node[sort[j]].arrive_time > node[sort[j+1]].arrive_time){
                int tmp = sort[j];
                sort[j] = sort[j+1];
                sort[j+1] = tmp;
                flag = 0;
            }
        }
        if(flag == 1){
            break;
        }
    }

    //线程运行顺序进队
    for(int i = 0;i < num;i++){
        In_Queue(queue,sort[i]);
    }

    //计算平均等待时间
    double average_waiting_time = Average_waiting_time(sort,num);
    return average_waiting_time;
}

double SJF(thread_queue *queue,int num){
    InitQueue(queue);

    //记录调整后的结点顺序
    int sort[max_thread_num];
    for(int i = 0;i < num;i++){
        sort[i] = i;
    }

    //按照线程到达时间从小到大排序
    for(int i = 0;i < num-1;i++){
        int flag = 1;
        for(int j = 0;j < num-1-i;j++){
            if(node[sort[j]].arrive_time > node[sort[j+1]].arrive_time){
                int tmp = sort[j];
                sort[j] = sort[j+1];
                sort[j+1] = tmp;
                flag = 0;
            }
        }
        if(flag == 1){
            break;
        }
    }

    //按照线程实时运行时间和线程要消耗时间进行排序
    //记录第一个线程的运行结束时间
    int end_time = node[sort[0]].arrive_time + node[sort[0]].serve_time;
    for(int i = 1;i < num;i++){
        int j;
        //找到线程运行结束时间前所有已经到达的线程
        for(j = i;j < num;j++){
            if(node[sort[j]].arrive_time > end_time){
                break;
            }
        }

        //对这些已经到达却还未运行线程，根据线程要消耗时间从小到大进行排序
        for(int k = i,x = 0;k < j-1;k++,x++){
            int flag = 1;
            for(int y = i;y < j-1-x;y++){
                if(node[sort[y]].serve_time > node[sort[y+1]].serve_time){
                    int tmp = sort[y];
                    sort[y] = sort[y+1];
                    sort[y+1] = tmp;
                    flag = 0;
                }
            }
            if(flag == 1){
                break;
            }
        }

        //更新下一个线程运行结束时间
        if(j == i){//前一个线程结束前，没有任何线程到达
            end_time = node[sort[i]].arrive_time + node[sort[i]].serve_time;
        }
        else{//前一个线程结束前，有线程到达
            end_time += node[sort[i]].serve_time;
        }
    }

    //线程运行顺序进队
    for(int i = 0;i < num;i++){
        In_Queue(queue,sort[i]);
    }

    //计算平均等待时间
    double average_waiting_time = Average_waiting_time(sort,num);
    return average_waiting_time;
}

double Priority(thread_queue *queue,int num){
    InitQueue(queue);

    //记录调整后的结点顺序
    int sort[max_thread_num];
    for(int i = 0;i < num;i++){
        sort[i] = i;
    }

    //按照线程到达时间从小到大排序
    for(int i = 0;i < num-1;i++){
        int flag = 1;
        for(int j = 0;j < num-1-i;j++){
            if(node[sort[j]].arrive_time > node[sort[j+1]].arrive_time){
                int tmp = sort[j];
                sort[j] = sort[j+1];
                sort[j+1] = tmp;
                flag = 0;
            }
        }
        if(flag == 1){
            break;
        }
    }

    //按照线程实时运行时间和优先级进行排序
    //记录第一个线程的运行结束时间
    int end_time = node[sort[0]].arrive_time + node[sort[0]].serve_time;
    for(int i = 1;i < num;i++){
        int j;
        //找到线程运行结束时间前所有已经到达的线程
        for(j = i;j < num;j++){
            if(node[sort[j]].arrive_time > end_time){
                break;
            }
        }

        //对这些已经到达却还未运行线程，根据优先级从小到大进行排序
        for(int k = i,x = 0;k < j-1;k++,x++){
            int flag = 1;
            for(int y = i;y < j-1-x;y++){
                if(node[sort[y]].priority > node[sort[y+1]].priority){
                    int tmp = sort[y];
                    sort[y] = sort[y+1];
                    sort[y+1] = tmp;
                    flag = 0;
                }
            }
            if(flag == 1){
                break;
            }
        }

        //更新下一个线程运行结束时间
        if(j == i){//前一个线程结束前，没有任何线程到达
            end_time = node[sort[i]].arrive_time + node[sort[i]].serve_time;
        }
        else{//前一个线程结束前，有线程到达
            end_time += node[sort[i]].serve_time;
        }
    }

    //线程运行顺序进队
    for(int i = 0;i < num;i++){
        In_Queue(queue,sort[i]);
    }

    //计算平均等待时间
    double average_waiting_time = Average_waiting_time(sort,num);
    return average_waiting_time;
}

//线程调度器，线程按照调度后的顺序进行逐个唤醒
void thread_dispatch(thread_queue *queue,int num){
    sleep(1);
    for(int i = 0;i < num;i++){
        //出队，获取要唤醒的线程的号码
        int thread_num = Out_Queue(queue);

        //对该线程上锁
        pthread_mutex_lock(&(node[thread_num].mutex)); 

        //唤醒该线程
        if(node[thread_num].state == block){
            node[thread_num].state = run;
            pthread_cond_signal(&(node[thread_num].pcond));
        }
        sleep(1);

        //对该线程解锁
        pthread_mutex_unlock(&(node[thread_num].mutex)); 
    }
}

//线程运行函数
static void *ftn(void *arg){
    int *s_addr = (int *)arg;
    int thread_num = *s_addr;

    //对该线程上锁
    pthread_mutex_lock(&(node[thread_num].mutex));

    //之间将该线程睡眠，等到线程调度器的唤醒
    if(node[thread_num].state == ready){
        node[thread_num].state = block;  
        pthread_cond_wait(&(node[thread_num].pcond), &(node[thread_num].mutex)); 
    }

    //线程调度器将该线程唤醒后，线程进入运行态
    if(node[thread_num].state == run){
        //获得当前开始运行的时间，计算下一个开始运行的时间
        if(node[thread_num].arrive_time <= next_start_time){
            start_time = next_start_time;
            next_start_time += node[thread_num].serve_time;
        }
        else{
            start_time = node[thread_num].arrive_time;
            next_start_time = node[thread_num].arrive_time + node[thread_num].serve_time;
        }
        printf("now is %d: thread %d start work,use time %d\n",start_time,thread_num,node[thread_num].serve_time);
    }

    //对该线程解锁
    pthread_mutex_unlock(&(node[thread_num].mutex));  
    pthread_exit(0);
}

int main(int argc, char *argv[]){
    int num;
    int ret;
    pthread_t ptid[num];
    int thread_num[max_thread_num];
    double average_waiting_time;

    //输入线程相关信息
    if (argc < 1) {
        printf("Usage: ./a.out FCFS / SJB / Priority\n");
        return 0;
    }
    printf("input the number of thread: ");
    scanf("%d",&num);
    if(num > max_thread_num){
        printf("num < %d\n",max_thread_num);
        return 0;
    }
    printf("input the (arrive_time serve_time priority) of thread\n");
    for (int i = 0; i < num; i++) {
        printf("thread %d: ",i);
        scanf("%d %d %d",&node[i].arrive_time, &node[i].serve_time, &node[i].priority);
        node[i].state = ready;
        pthread_cond_init(&(node[i].pcond), NULL);
		pthread_mutex_init(&(node[i].mutex), NULL);
    }

    //根据调度策略，对线程进行排序
    thread_queue queue;
    if (strcmp(argv[1],"FCFS") == 0) {
        average_waiting_time = FCFS(&queue,num);
    }
    else if (strcmp(argv[1],"SJF") == 0) {
        average_waiting_time = SJF(&queue,num);
    }
    else if (strcmp(argv[1],"Priority") == 0) {
        average_waiting_time = Priority(&queue,num);
    }

    //创建线程，并将线程进入睡眠
    for (int i = 0; i < num; i++) {
        thread_num[i] = i;
    }
    for (int i = 0; i < num; i++) {
        ret = pthread_create(&ptid[i], NULL, &ftn, (void *)&thread_num[i]);
        if(ret != 0) {
            fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
            exit(1);
        }
    }

    //线程调度器根据调度策略排序后的顺序，对线程进行唤醒
    thread_dispatch(&queue,num);

    for (int i = 0; i < num; i++) {
        ret = pthread_join(ptid[i], NULL);
        if(ret != 0) {
            fprintf(stderr, "pthread_join error: %s\n", strerror(ret));
            exit(1);
        }
    }
    printf("平均等待时间为： %.2lf\n",average_waiting_time);
    
    return 0;
}

