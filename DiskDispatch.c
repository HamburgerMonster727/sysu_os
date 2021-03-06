#include <stdio.h>
#include <string.h>

#define max_cylinder 200         //柱面最大值
#define max_request 20

int dispatch[max_request + 2];   //调度次序
int cylinder[max_request];       //要进行调度的柱面
int request_num;                 //调度个数
int start_cylinder;              //初始柱面
double average_length = 0;       //平均寻找长度

void FCFS(){
    int start = start_cylinder;

    for(int i = 0;i < request_num;i++){
        //按照FIFO顺序进行调度
        dispatch[i] = cylinder[i];
        //计算距离
        int distance = start_cylinder - dispatch[i];
        if(distance < 0){
            distance = 0 - distance;
        }
        average_length += distance;
        //更改初始柱面的值
        start_cylinder = dispatch[i];
    }

    printf("Disk dispatch: ");
    printf("%d->%d  ",start,dispatch[0]);
    for(int i = 0;i < request_num - 1;i++){
        printf("%d->%d  ",dispatch[i],dispatch[i+1]);
    }
    printf("\n");
    average_length = average_length / request_num;
    printf("average seek length: %0.2lf\n",average_length);
}

void SSTF(){
    int start = start_cylinder;

    for(int i = 0;i < request_num;i++){
        //找到距离初始柱面距离最近的点
        int min_length = 2 * max_cylinder;
        int min_place;
        for(int j = 0;j < request_num;j++){
            int length = start_cylinder - cylinder[j];
            if(length < 0){
                length = 0 - length;
            }
            if(length < min_length){
                min_length = length;
                min_place = j;
            }
        }
        dispatch[i] = cylinder[min_place];

        //计算距离
        int distance = start_cylinder - dispatch[i];
        if(distance < 0){
            distance = 0 - distance;
        }
        average_length += distance;
        //更改初始柱面的值
        start_cylinder = dispatch[i];
        //将当前结点的值设置为最大，这样寻找最小距离点时不会被调度到
        cylinder[min_place] = 2 * max_cylinder;
    }

    printf("Disk dispatch: ");
    printf("%d->%d  ",start,dispatch[0]);
    for(int i = 0;i < request_num - 1;i++){
        printf("%d->%d  ",dispatch[i],dispatch[i + 1]);
    }
    printf("\n");
    average_length = average_length / request_num;
    printf("average seek length: %0.2lf\n",average_length);
}

void SCAN(){
    int start = start_cylinder;
    int elevator[max_cylinder];  //利用电梯来实现SCAN算法

    //初始化电梯的值，电梯需要被调度的，值为1，否则为-1
    for(int i = 0;i < max_cylinder;i++){
        elevator[i] = -1;
    }
    for(int i = 0;i < request_num;i++){
        elevator[cylinder[i]] = 1;
    }

    //从初始柱面开始，电梯往下走
    int j = 0;
    for(int i = start_cylinder;i >= 0;i--){
        //当电梯为1时，进行调度
        if(elevator[i] == 1){
            dispatch[j] = i;
            //计算距离
            int distance = start_cylinder - dispatch[j];
            average_length += distance;
            start_cylinder = dispatch[j];
            //电梯值改为-1，调度完毕
            elevator[i] = -1;
            j++;
        }
    }
    
    //电梯走到0，初始柱面修改为0
    average_length += start_cylinder;
    start_cylinder = 0;
    dispatch[j] = 0;
    j++;

    //电梯从0开始往上走
    for(int i = start_cylinder;i < max_cylinder;i++){
        //当电梯为1时，进行调度
        if(elevator[i] == 1){
            dispatch[j] = i;
            //计算距离
            int distance = dispatch[j] - start_cylinder;
            average_length += distance;
            start_cylinder = dispatch[j];
            //电梯值改为-1，调度完毕
            elevator[i] = -1;
            j++;
        }
    }

    printf("Disk dispatch: ");
    printf("%d->%d  ",start,dispatch[0]);
    for(int i = 0;i < request_num;i++){
        printf("%d->%d  ",dispatch[i],dispatch[i + 1]);
    }
    printf("\n");
    average_length = average_length / request_num;
    printf("average seek length: %0.2lf\n",average_length);
}

void C_SCAN(){
    int start = start_cylinder;
    int elevator[max_cylinder]; //利用电梯来实现C_SCAN算法

    //初始化电梯的值，电梯需要被调度的，值为1，否则为-1
    for(int i = 0;i < max_cylinder;i++){
        elevator[i] = -1;
    }
    for(int i = 0;i < request_num;i++){
        elevator[cylinder[i]] = 1;
    }

    //从初始柱面开始，电梯往上走，直到最大柱面
    int j = 0;
    for(int i = start_cylinder;i < max_cylinder;i++){
        if(elevator[i] == 1){
            dispatch[j] = i;
            int distance = dispatch[j] - start_cylinder;
            average_length += distance;
            start_cylinder = dispatch[j];
            elevator[i] = -1;
            j++;
        }
    }

    //电梯走到最大柱面，初始柱面修改为最大柱面值
    dispatch[j] = max_cylinder - 1;
    average_length += dispatch[j] - start_cylinder;
    start_cylinder = dispatch[j];
    j++;

    //电梯从最大柱面走到最小柱面，初始柱面修改为0
    dispatch[j] = 0;
    average_length += start_cylinder - dispatch[j];
    start_cylinder = 0;
    j++;
    
    //电梯从0开始往上走，直到所有点都被调度完
    for(int i = start_cylinder;j < request_num+2;i++){
        if(elevator[i] == 1){
            dispatch[j] = i;
            int distance = dispatch[j] - start_cylinder;
            average_length += distance;
            start_cylinder = dispatch[j];
            elevator[i] = -1;
            j++;
        }
    }

    printf("Disk dispatch: ");
    printf("%d->%d  ",start,dispatch[0]);
    for(int i = 0;i < request_num + 1;i++){
        printf("%d->%d  ",dispatch[i],dispatch[i + 1]);
    }
    printf("\n");
    average_length = average_length / request_num;
    printf("average seek length: %0.2lf\n",average_length);
}

void LOOK(){
    int start = start_cylinder;
    int elevator[max_cylinder]; //利用电梯来实现LOOK算法

    //初始化电梯的值，电梯需要被调度的，值为1，否则为-1
    for(int i = 0;i < max_cylinder;i++){
        elevator[i] = -1;
    }
    for(int i = 0;i < request_num;i++){
        elevator[cylinder[i]] = 1;
    }

    //从初始柱面开始，电梯往下走
    int j = 0;
    for(int i = start_cylinder;i >= 0;i--){
        if(elevator[i] == 1){
            dispatch[j] = i;
            int distance = start_cylinder - dispatch[j];
            average_length += distance;
            start_cylinder = dispatch[j];
            elevator[i] = -1;
            j++;

            //判断继续走是否还有点需要被调度
            int flag = 0;
            for(int k = i;k >= 0;k--){
                if(elevator[k] == 1){
                    flag = 1;
                    break;
                }
            }
            //当继续走没有点要被调度时，电梯停下
            if(flag == 0){
                break;
            }
        }
    }

    //电梯开始往上走
    for(int i = start_cylinder;i < max_cylinder;i++){
        if(elevator[i] == 1){
            dispatch[j] = i;
            int distance = dispatch[j] - start_cylinder;
            average_length += distance;
            start_cylinder = dispatch[j];
            elevator[i] = -1;
            j++;

            //判断继续走是否还有点需要被调度
            int flag = 0;
            for(int k = i;k < max_cylinder;k++){
                if(elevator[k] == 1){
                    flag = 1;
                    break;
                }
            }
            //当继续走没有点要被调度时，电梯停下
            if(flag == 0){
                break;
            }
        }
    }

    printf("Disk dispatch: ");
    printf("%d->%d  ",start,dispatch[0]);
    for(int i = 0;i < request_num - 1;i++){
        printf("%d->%d  ",dispatch[i],dispatch[i + 1]);
    }
    printf("\n");
    average_length = average_length / request_num;
    printf("average seek length: %0.2lf\n",average_length);
}

void C_LOOK(){
    int start = start_cylinder;
    int elevator[max_cylinder]; //利用电梯实现C_LOOK算法

    //初始化电梯的值，电梯需要被调度的，值为1，否则为-1
    for(int i = 0;i < max_cylinder;i++){
        elevator[i] = -1;
    }
    for(int i = 0;i < request_num;i++){
        elevator[cylinder[i]] = 1;
    }

    //从初始柱面开始，电梯往上走
    int j = 0;
    for(int i = start_cylinder;i < max_cylinder;i++){
        if(elevator[i] == 1){
            dispatch[j] = i;
            int distance = dispatch[j] - start_cylinder;
            average_length += distance;
            start_cylinder = dispatch[j];
            elevator[i] = -1;
            j++;

            //判断继续走是否还有点需要被调度
            int flag = 0;
            for(int k = i;k < max_cylinder;k++){
                if(elevator[k] == 1){
                    flag = 1;
                    break;
                }
            }
            //当继续走没有点要被调度时，电梯停下
            if(flag == 0){
                break;
            }
        }
    }
    
    //循环，找到最小的要被调度的柱面，修改初始柱面的值
    for(int i = 0;i < max_cylinder;i++){
        if(elevator[i] == 1){
            dispatch[j] = i;
            int distance = start_cylinder - dispatch[j];
            average_length += distance;
            start_cylinder = dispatch[j];
            elevator[i] = -1;
            j++;
            break;
        }
    }
    
    //电梯继续往上走
    for(int i = start_cylinder;j < request_num;i++){
        if(elevator[i] == 1){
            dispatch[j] = i;
            int distance = dispatch[j] - start_cylinder;
            average_length += distance;
            start_cylinder = dispatch[j];
            elevator[i] = -1;
            j++;

            //判断继续走是否还有点需要被调度
            int flag = 0;
            for(int k = i;k < max_cylinder;k++){
                if(elevator[k] == 1){
                    flag = 1;
                    break;
                }
            }
            //当继续走没有点要被调度时，电梯停下
            if(flag == 0){
                break;
            }
        }
    }

    printf("Disk dispatch: ");
    printf("%d->%d  ",start,dispatch[0]);
    for(int i = 0;i < request_num - 1;i++){
        printf("%d->%d  ",dispatch[i],dispatch[i + 1]);
    }
    printf("\n");
    average_length = average_length / request_num;
    printf("average seek length: %0.2lf\n",average_length);
}

int main(int argc, char *argv[]){
    if (argc < 1) {
        printf("Usage: ./a.out FCFS / SSTF / SCAN / C_SCAN / LOOK / C_LOOK \n");
        return 0;
    }

    printf("input the start cylinder: ");
    scanf("%d",&start_cylinder);
    printf("input the number of request for cylinders: ");
    scanf("%d",&request_num);
    printf("input the cylinders queue: ");
    for(int i = 0;i < request_num;i++){
        scanf("%d",&cylinder[i]);
    }

    if (strcmp(argv[1],"FCFS") == 0) {
        FCFS();
    }
    else if (strcmp(argv[1],"SSTF") == 0) {
        SSTF();
    }
    else if (strcmp(argv[1],"SCAN") == 0) {
        SCAN();
    }
    else if (strcmp(argv[1],"C_SCAN") == 0) {
        C_SCAN();
    }
    else if (strcmp(argv[1],"LOOK") == 0) {
        LOOK();
    }
    else if (strcmp(argv[1],"C_LOOK") == 0) {
        C_LOOK();
    }

    return 0;
}