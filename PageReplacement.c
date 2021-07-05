#include <stdio.h>
#include <math.h>
#include <string.h>

#define max_frame_num 10
#define max_page_num 30

int page_num;               //页面数量
int frame_num;              //块数量
int page[max_page_num];
int frame[max_frame_num];
int fault_num = 0;          //页面命中失败次数

void FIFO(){
    int front = -1;//用于遍历循环队列

    //初始化页面块
    for(int i = 0;i < frame_num;i++){
        frame[i] = -1;
    }

    for(int i = 0;i < page_num;i++){
        //判断页面是否命中
        int is_fault = 1; 
        for(int j = 0;j < frame_num;j++){
            if(frame[j] == page[i]){
                is_fault = 0;
                break;
            }
        }

        //页面未命中时
        if(is_fault == 1){
            //循环队列
            front = (front + 1) % frame_num;
            frame[front] = page[i];
            fault_num++;
            printf("page falut: %d   ",page[i]);
        } 
        
        //页面命中时
        if(is_fault == 0){
            printf("page hit  : %d   ",page[i]);
        }

        //输出页面块
        for(int j = 0;j < frame_num;j++){
            if(frame[j] != -1){
                printf("%d ",frame[j]);
            }
        }
        printf("\n");
    }

    printf("total %d page faluts\n",fault_num);
}

void LRU(){ 
    int time[frame_num];//利用时间来辅助LRU

    //初始化页面块和时间，将时间从大到小进行初始化
    for(int i = 0;i < frame_num;i++){
        frame[i] = -1;
        time[i] = frame_num - i;
    }

    for(int i = 0;i < page_num;i++){
        //判断页面是否命中
        int j;
        int is_fault = 1; 
        for(j = 0;j < frame_num;j++){
            if(frame[j] == page[i]){
                is_fault = 0;
                break;
            }
        }

        //页面未命中时
        if(is_fault == 1){
            //找到最长时间未使用的块
            int longest_time = 0;
            for(int k = 0;k < frame_num;k++){
                if(time[longest_time] < time[k]){
                    longest_time = k;
                }
            }
            //将该块的时间初始化为0
            frame[longest_time] = page[i];
            time[longest_time] = 0;
            fault_num++;
            printf("page falut: %d   ",page[i]);
        }
        
        //页面命中时，将命中的块的时间初始化为0
        if(is_fault == 0){
            time[j] = 0;
            printf("page hit  : %d   ",page[i]);
        }

        //输出页面块并且对时间进行处理
        for(int k = 0;k < frame_num;k++){
            if(frame[k] != -1){
                printf("%d ",frame[k]);
            }
            //所有块的时间+1
            time[k]++;
        }
        printf("\n");
    }

    printf("total %d page faluts\n",fault_num);
}

void LRU_matrix(){
    int sum[max_frame_num];//记录矩阵每一行的二进制大小
    int matrix[max_frame_num][max_frame_num];//利用矩阵来辅助LRU

    //初始化页面块和矩阵
    for(int i = 0;i < frame_num;i++){
        frame[i] = -1;
    }
    for(int i = 0;i < frame_num;i++){
        for(int j = 0;j < frame_num;j++){
            matrix[i][j] = 0;
        }
    }

    for(int i = 0;i < page_num;i++){
        //判断页面是否命中
        int j;
        int is_fault = 1; 
        int min_row = 0;
        for(j = 0;j < frame_num;j++){
            if(frame[j] == page[i]){
                is_fault = 0;
                break;
            }
        }

        //页面未命中时
        if(is_fault == 1){
            //初始化矩阵每一行的值
            for(int k = 0;k < frame_num;k++){
                sum[k] = 0;
            }
            //计算矩阵每一行的二进制值
            for(int k = 0;k < frame_num;k++){
                for(int p = frame_num - 1,q = 0;p >= 0;p--,q++){
                    sum[k] += matrix[k][p]*pow(2,q);
                }
            }
            //找到矩阵中二级制值最小的那一行
            for(int k = 0;k < frame_num;k++){
                if(sum[min_row] > sum[k]){
                    min_row = k;
                }
            }

            frame[min_row] = page[i];
            fault_num++;

            //第min_row行全部先置为1，然后第min_row列全部置为0
            for(int k = 0;k < frame_num;k++){
                matrix[min_row][k] = 1;
            }
            for(int k = 0;k < frame_num;k++){
                matrix[k][min_row] = 0;
            }
            printf("page falut: %d   ",page[i]);
        }

        //页面命中时
        if(is_fault == 0){
            //第j行全部先置为1，然后第j列全部置为0
            for(int k = 0;k < frame_num;k++){
                matrix[j][k] = 1;
            }
            for(int k = 0;k < frame_num;k++){
                matrix[k][j] = 0;
            }
            printf("page hit  : %d   ",page[i]);
        }

        //输出页面块
        for(int k = 0;k < frame_num;k++){
            if(frame[k] != -1){
                printf("%d ",frame[k]);
            }
        }
        printf("\n");
    }

    printf("total %d page faluts\n",fault_num);
}

void SecondChance(){
    int clock_point = 0;//利用时间指针来辅助SecondChance
    int use[max_page_num];//判断该块是否被use
    
    //初始化页面块和use值
    for(int i = 0;i < frame_num;i++){
        frame[i] = -1;
        use[i] = 0;
    }

    for(int i = 0;i < page_num;i++){
        //判断页面是否命中
        int j;
        int is_fault = 1; 
        for(j = 0;j < frame_num;j++){
            if(frame[j] == page[i]){
                is_fault = 0;
                break;
            }
        }

        //页面未命中时
        if(is_fault == 1){
            //无限循环，直到该页面被分配
            while(1){
                //当clock_point指向的块的use值为0时，将页面分配到该块，use置为1
                if(use[clock_point] == 0){
                    frame[clock_point] = page[i];
                    use[clock_point] = 1;
                    //clock_point指向下一块
                    clock_point = (clock_point + 1) % frame_num;
                    break;
                }

                //当clock_point指向的块的use值为1时，use置为0
                if(use[clock_point] == 1){
                    use[clock_point] = 0;
                    //clock_point指向下一块
                    clock_point = (clock_point + 1) % frame_num;
                }
            }

            fault_num++;
            printf("page falut: %d   ",page[i]);
        }

        //页面命中时,clock_point无需移动，将命中的块的use置为1
        if(is_fault == 0){
            use[j] = 1;
            printf("page hit  : %d   ",page[i]);
        }

        //输出页面块
        for(int k = 0;k < frame_num;k++){
            if(frame[k] != -1){
                printf("%d ",frame[k]);
            }
        }
        printf("\n");
    }

    printf("total %d page faluts\n",fault_num);
}

int main(int argc, char *argv[]){
    if (argc < 1) {
        printf("Usage: ./a.out FIFO / LRU / LRU_matrix / SecondChance\n");
        return 0;
    }

    printf("input the number of frame: ");
    scanf("%d",&frame_num);
    printf("input the number of page: ");
    scanf("%d",&page_num);
    printf("input the page: ");
    for(int i = 0;i < page_num;i++){
        scanf("%d",&page[i]);
    }

    if (strcmp(argv[1],"FIFO") == 0) {
        FIFO();
    }
    else if (strcmp(argv[1],"LRU") == 0) {
        LRU();
    }
    else if (strcmp(argv[1],"LRU_matrix") == 0) {
        LRU_matrix();
    }
    else if (strcmp(argv[1],"SecondChance") == 0) {
        SecondChance();
    }

    return 0;
}