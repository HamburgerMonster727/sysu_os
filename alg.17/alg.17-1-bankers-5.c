#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
#define N 5  /* number of processes */
#define M 3  /* number of resource types */

int resource[M] =         {10, 5, 7};
int max[N][M] =          {{ 7, 5, 3}, {3, 2, 2}, {9, 0, 2}, {2, 2, 2}, {4, 3, 3}};
int allocation[N][M]; /* {{ 0, 1, 0}, {2, 0, 0}, {3, 0, 2}, {2, 1, 1}, {0, 0, 2}}; */
int need[N][M];       /* {{ 7, 4, 3}, {1, 2, 2}, {6, 0, 0}, {0, 1, 1}, {4, 3, 1}}; */
int available[M];     /*  { 3, 3, 2}; */
int request[M] =          { 0, 0, 0};

int work[M]; 

void print_stat(void)
{
    int j, k;

    printf(" available\t need\t\t allocation\n");
    
    for (k = 0;  k < M; k++)
        printf("%2d,", available[k]);
    printf("\t");
    j = 0;
    for (k = 0;  k < M; k++)
        printf("%2d,", need[j][k]);
    printf("\t");
    for (k = 0;  k < M; k++)
        printf("%2d,", allocation[j][k]);
    printf("\n");
    for (j = 1;  j < N; j++) {
        printf("\t\t");
        for (k = 0;  k < M; k++)
            printf("%2d,", need[j][k]);
        printf("\t");
        for (k = 0;  k < M; k++)
            printf("%2d,", allocation[j][k]);
        printf("\n");
    }

    return;
}


int safecheck(int pro_i)
{
    int finish[N] = {0};
    int safe_seq[N] = {0};
    int i, j, k, l;

    printf("==== safecheck starting ====\n");

    for (i = 0; i < N; i++) { /* we need to select N processes, i is a counter */
        for (j = 0; j < N; j++) { /* check the j-th process */ 
            if(finish[j] == FALSE) {
                if(j == pro_i) {
                    for (k = 0; k < M; k++) {
                        if(need[pro_i][k] - request[k] > work[k]) { /* if pretending need[pro_i] > work[] */
                            break; /* to check next process */
                        }
                    } 
                } else {
                    for (k = 0; k < M; k++) {
                        if(need[j][k] > work[k]) {
                            break; /* to check next process */
                        }
                    }
                }

                if(k == M) { /* the j-th process can finish its task */
                    safe_seq[i] = j;
                    finish[j] = TRUE;
                    printf("safe_seq[%d] = %d\n", i, j);
                    printf("new work vector: ");
                    if(j == pro_i) {
                        for (l = 0; l < M; l++) { /* process pro_i releasing pretending allocated resources */
                            work[l] += allocation[pro_i][l] + request[l];
                            printf("%d, ", work[l]);
                        }
                    } else {
                        for (l = 0; l < M; l++) { /* other process releasing allocated resource */
                            work[l] += allocation[j][l];
                            printf("%d, ", work[l]);
                        }
                    }
                    printf("\n");

                    break; /* to select more process */
                }
            }
        }

        if(j == N) {
            break; /* not enough processes can pass the safecheck */
        }
    }

    if(i == N) { /* all N processes passed the safecheck */
        printf("A safty sequence is: ");
        for (j = 0; j < N; j++) {
            printf("P%d, ", safe_seq[j]);
        }
        printf("\n");
        return EXIT_SUCCESS;
    }
    else {
        printf("safecheck failed, process %d suspended\n", pro_i);
        return EXIT_FAILURE;
    }
}


int resource_request(int pro_i)
{
    int j, k, ret;
    
    for (k = 0;  k < M; k++) {
        if(request[k] > need[pro_i][k]) {
            printf("request[k] > need[pro_i][k], k = %d, request[k] = %d, need[pro_i][k] = %d\n", k, request[k], need[pro_i][k]);
            return EXIT_FAILURE;
        }
    }

    for (k = 0; k < M; k++) {
        if(request[k] > available[k]) {
            printf("request[k] > available[k], k = %d, request[k] = %d, available[k] = %d\n", k, request[k], available[k]);
            return EXIT_FAILURE;
        }
    }

    for (k = 0; k < M; k++) {
        work[k] = available[k] - request[k]; /* work[] as the pretending available[] */
    }

    ret = safecheck(pro_i); /* check whether the pretending state is safe */

    if(ret == EXIT_SUCCESS) { /* confirm the pretending state as the new state */
        for (k = 0; k < M; k++) {
            available[k] -= request[k];
            need[pro_i][k] -= request[k];
            allocation[pro_i][k] += request[k];
        }
    }

    return ret;
}


void main(void)
{
    int pro_i;
    int j, k, ret;

    for (j = 0;  j < N; j++)
        for (k = 0;  k < M; k++)
            need[j][k] = max[j][k] - allocation[j][k];

    for (k = 0;  k < M; k++)
        available[k] = resource[k];
    for (j = 0;  j < N; j++)
        for (k = 0;  k < M; k++)
            available[k] -= allocation[j][k];

    printf("==== initial state ====\n");
    print_stat();

    while (1) {
        printf("input process number (0 - %d, -1 quit): ", j = N-1);
        scanf("%d", &pro_i);
        if (pro_i < 0)
            break;
        if (pro_i >= N) {
            printf("pro_i out of range\n");
            continue;
        }
        printf("input request vectore of lemgth %d, separated by a space: ", k = M);
        scanf("%d %d %d", &request[0], &request[1], &request[2]);
        
        printf("pro_i = %d\n", pro_i);
        printf("request = ");
        for (k = 0; k < M; k++)
            printf("%d, ", request[k]);
        printf("\n");
        
        ret = resource_request(pro_i);
        if (ret == EXIT_SUCCESS) {
            printf("==== new state ====\n");
            print_stat();
        }
    }

    return;
}
