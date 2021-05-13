#include <string.h>
#define HEAP_NUM 20    
#define NAME_SIZE 10
#define true 1
#define false 0

#define PERM S_IRUSR|S_IWUSR|IPC_CREAT

#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while(0)

typedef struct Heap{
    int flag;
    int no;
    char name[NAME_SIZE];
}Heap;

typedef struct shared_struct{
    int lock;
    int heap_num;
    Heap heap[HEAP_NUM];
}shared_struct;

void HeapSortAdjust(Heap heap[],int start,int end){ 
    int tmpno = heap[start].no;
    int tmpflag = heap[start].flag;
    char tmpname[NAME_SIZE];
    strcpy(tmpname,heap[start].name);
    for(int i = 2*start;i <= end;i *= 2){
        if(i+1 <= end && heap[i].no > heap[i+1].no){
            i++; 
        } 
        if(heap[i].no > tmpno){
            break; 
        } 
        heap[start].no = heap[i].no; 
        heap[start].flag = heap[i].flag;
        strcpy(heap[start].name,heap[i].name);
        start = i; 
    } 
    heap[start].no = tmpno; 
    heap[start].flag = tmpflag;
    strcpy(heap[start].name,tmpname);
} 

void swap(Heap heap[],int i,int j){
    int tmp; 
    tmp = heap[i].no; 
    heap[i].no = heap[j].no; 
    heap[j].no = tmp; 
    char tmpname[NAME_SIZE];
    strcpy(tmpname,heap[i].name);
    strcpy(heap[i].name,heap[j].name);
    strcpy(heap[j].name,tmpname);
    tmp = heap[i].flag;
    heap[i].flag = heap[j].flag; 
    heap[j].flag = tmp;
} 

void Push(shared_struct *shared,Heap heap[],int no_,char name_[]){  
    if(shared->heap_num == HEAP_NUM){
        return;
    }
    shared->heap_num++;
    heap[shared->heap_num].no = no_;
    heap[shared->heap_num].flag = true;
    strcpy(heap[shared->heap_num].name,name_);
    for(int m = shared->heap_num/2;m > 0;m /= 2){
        HeapSortAdjust(heap,m,shared->heap_num);
    }
}

void Pop(shared_struct *shared,Heap heap[]){
    if(shared->heap_num == 0){
        return;
    }
    printf("学号：%d 姓名：%s\n",shared->heap[1].no,shared->heap[1].name);
    heap[1].flag = false;
    swap(heap,1,shared->heap_num);
    shared->heap_num--;
    HeapSortAdjust(heap,1,shared->heap_num);
}

int Search(shared_struct *shared,int no_){
    for(int i = 1;i <= shared->heap_num;i++){
        if(shared->heap[i].no == no_)
            return i;
    }
    printf("未找到所需学号\n");
    return -1;
}

void Change(shared_struct *shared){
    printf("输入你想修改的学号：");
    int no_;
    scanf("%d",&no_);
    int n = Search(shared,no_);
    if(n == -1){
        return;
    }
    printf("输入修改后的学号：");
    scanf("%d",&no_);
    shared->heap[n].no = no_;
    printf("输入修改后的姓名：");
    char name_[NAME_SIZE];
    scanf("%s",name_);
    strcpy(shared->heap[n].name,name_);
}

void Show(shared_struct *shared){
    for(int i = 1;i <= shared->heap_num;i++){
        printf("学号：%d 姓名：%s\n",shared->heap[i].no,shared->heap[i].name);
    }
}

void Sort(shared_struct *shared,Heap heap[]){ 
    int n = shared->heap_num;
    for(int m = n/2;m > 0;m--){
        HeapSortAdjust(heap,m,n); 
    } 
} 