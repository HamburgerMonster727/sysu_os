#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#define MAX_N 1024

static int counter = 0;
  /* number of process(s) in the critical section */
int level[MAX_N];
  /* level number of processes 0 .. MAX_N-1 */
int waiting[MAX_N-1];
  /* waiting process of each level number 0 .. MAX_N-2 */
int max_num = 20; /* default max thread number */

static void *ftn(void *arg){
    int *s_addr = (int *)arg;
    int thread_num = *s_addr;
    printf("i am thread number %d, ptid = %ld\n", thread_num, pthread_self());
    int lev, k;

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
    counter++;
    if(counter > 1){
        kill(getpid(),SIGKILL);
    }
    counter--;
    level[thread_num] = -1;
    pthread_exit(0);
}
 
int main(){ /* create some pthread threads */ 

    int ret;
    int order;
    int num[order];

    for (int i = 0; i < order; i++) {
        num[i] = i;
    }

    long start_us, end_us;
    struct timeval t;

    gettimeofday(&t, 0);
    start_us = (long)(t.tv_sec * 1000 * 1000) + t.tv_usec;

    sleep(2);

    gettimeofday(&t, 0);
    end_us = (long)(t.tv_sec * 1000 * 1000) + t.tv_usec;

    printf("time testing... sleeping time = %ld micro sec\n", end_us - start_us);
    	    
    gettimeofday(&t, 0);
    start_us = (long)(t.tv_sec * 1000 * 1000) + t.tv_usec;

    pthread_t ptid[order];
    for (int i = 0; i < order; i++) {
        ret = pthread_create(&ptid[i], NULL, &ftn, (void *)&num[i]);
        if(ret != 0) {
            fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
            exit(1);
        }
    }

    for (int i = 0; i < order; i++) {
        ret = pthread_join(ptid[i], NULL);
        if(ret != 0) {
            fprintf(stderr, "pthread_join error: %s\n", strerror(ret));
            exit(1);
        }
    }

    gettimeofday(&t, 0);
    end_us = (long)(t.tv_sec * 1000 * 1000) + t.tv_usec;
    printf("with %d threads, running time = %ld usec\n", order, end_us - start_us);

    return;
}

