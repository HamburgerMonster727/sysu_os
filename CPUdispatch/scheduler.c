#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_THREADS 32768

#define SCHED_TYPE_FCFS 0
#define SCHED_TYPE_SRTF 1
#define SCHED_TYPE_PBS 2
#define SCHED_TYPE_MLFQ 3

typedef struct tag_node
{
	struct tag_node* nextnode;
	int ID;
	int index;
	// remaining time or priority
	int value;
	// current time
	float subvalue;
}node;

static int __sched_type = -1;
static volatile int __time = 0;
static int __current_tid = -1;

static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t task_queue_cond[MAX_THREADS];
static pthread_mutex_t task_queue_mutex[MAX_THREADS];
static int queue_front = 0;
static int queue_back = 0;
static int headbechanged = 0;
static int insertlower = 0;
static int mlsqfirsttime = 1;
node* listhead = NULL;
node* multihead[5];
int level[50];
int tcount[50];
int qcount[5];

void init_scheduler(int sched_type);
int scheduleme(float currentTime, int tid, int remainingTime, int tprio);
void sched_fcfs(float currentTime, int tid, int remainingTime);
void sched_srtf(float currentTime, int tid, int remainingTime);
void sched_pbs(float currentTime, int tid, int remainingTime, int tprio);
void sched_mlfq(float currentTime, int tid, int remainingTime);
static node* enqueuenode(node*head, int ID, int index, int value, float subvalue);
static node* enqueuemulti(node*head, int ID, int index, int value, float subvalue);

void init_scheduler(int sched_type) {
	// check whether __sched_type is already set
	if (__sched_type != -1) {
		fprintf(stderr, "Scheduler already initialized!\n");
	}

	switch (sched_type) {
	case SCHED_TYPE_FCFS:
		__sched_type = sched_type;
		pthread_mutex_init(&queue_mutex, NULL);
		break;
	case SCHED_TYPE_SRTF:
		__sched_type = sched_type;
		pthread_mutex_init(&queue_mutex, NULL);
		break;
	case SCHED_TYPE_PBS:
		__sched_type = sched_type;
		pthread_mutex_init(&queue_mutex, NULL);
		break;
	case SCHED_TYPE_MLFQ:
		__sched_type = sched_type;
		pthread_mutex_init(&queue_mutex, NULL);
		break;
	default:
		fprintf(stderr, "Unimplemented scheduler type: %d\n", sched_type);
	}
}

int scheduleme(float currentTime, int tid, int remainingTime, int tprio) {
	pthread_mutex_lock(&queue_mutex);
	//fprintf(stderr, "scheduleme called: scheduleme(%f, %d, %d, %d)\n", currentTime, tid, remainingTime, tprio);

	__time = (int) ceil(currentTime);

	switch (__sched_type) {
	case SCHED_TYPE_FCFS:
		sched_fcfs(currentTime, tid, remainingTime);
		break;
	case SCHED_TYPE_SRTF:
		sched_srtf(currentTime, tid, remainingTime);
		break;
	case SCHED_TYPE_PBS:
		sched_pbs(currentTime, tid, remainingTime, tprio);
		break;
	case SCHED_TYPE_MLFQ:
		sched_mlfq(currentTime, tid, remainingTime);
		break;
	}
	return __time;
}

void sched_fcfs(float currentTime, int tid, int remainingTime) {
	if (remainingTime == 0) {
		pthread_mutex_unlock(&queue_mutex);
		// task complete, pick new task from queue
		if (++queue_front < queue_back) {
			// signals the next task if any
			pthread_cond_signal(&task_queue_cond[queue_front]);
		}
	}
	else if (tid == __current_tid) {
		// update on current task
		pthread_mutex_unlock(&queue_mutex);
	}
	else {
		// enqueue new task and block it
		int index = queue_back++;
		pthread_mutex_unlock(&queue_mutex);
		pthread_cond_init(&task_queue_cond[index], NULL);
		pthread_mutex_init(&task_queue_mutex[index], NULL);
		while (queue_front != index) {
			pthread_mutex_lock(&task_queue_mutex[index]);
			pthread_cond_wait(&task_queue_cond[index], &task_queue_mutex[index]);
			pthread_mutex_unlock(&task_queue_mutex[index]);
		}
		__current_tid = tid;
	}
}
void sched_srtf(float currentTime, int tid, int remainingTime) {
	if (remainingTime == 0) {
		//fprintf(stderr, "remainingTime = 0\n");
		if (!headbechanged){
			listhead = listhead->nextnode;
		}
		else{
			headbechanged = 0;
		}

		pthread_mutex_unlock(&queue_mutex);
		// task complete, pick new task from queue
		/*
		if(listhead == NULL){
		fprintf(stderr, "listhead = 0\n");
		}
		*/
		if (listhead != NULL) {
			__current_tid = listhead->ID;
			// signals the next task if any
			//fprintf(stderr, "Signaling: %d\n", listhead->ID);
			pthread_cond_signal(&task_queue_cond[listhead->index]);
		}
	}
	else if (headbechanged){
		//fprintf(stderr, "headbechanged = %d\n", listhead->ID);
		headbechanged = 0;
		pthread_mutex_unlock(&queue_mutex);
		__current_tid = listhead->ID;
		//fprintf(stderr, "Signaling: %d\n", listhead->ID);
		pthread_cond_signal(&task_queue_cond[listhead->index]);

		//fprintf(stderr, "enqueuing = %d\n", tid);
		// enqueue new task and block it
		int index = queue_back++;
		pthread_mutex_lock(&queue_mutex);
		listhead = enqueuenode(listhead, tid, index, remainingTime, currentTime);
		pthread_mutex_unlock(&queue_mutex);
		pthread_cond_init(&task_queue_cond[index], NULL);
		pthread_mutex_init(&task_queue_mutex[index], NULL);

		while (__current_tid != tid) {
			pthread_mutex_lock(&task_queue_mutex[index]);
			pthread_cond_wait(&task_queue_cond[index], &task_queue_mutex[index]);
			pthread_mutex_unlock(&task_queue_mutex[index]);
		}
	}
	else if (tid == __current_tid){
		listhead->value = remainingTime;
		listhead->subvalue = currentTime;
		pthread_mutex_unlock(&queue_mutex);
	}
	else{
		if (listhead == NULL){
			__current_tid = tid;
		}
		//fprintf(stderr, "enqueuing = %d\n", tid);
		// enqueue new task and block it
		int index = queue_back++;
		listhead = enqueuenode(listhead, tid, index, remainingTime, currentTime);
		pthread_mutex_unlock(&queue_mutex);
		pthread_cond_init(&task_queue_cond[index], NULL);
		pthread_mutex_init(&task_queue_mutex[index], NULL);

		while (__current_tid != tid) {
			pthread_mutex_lock(&task_queue_mutex[index]);
			pthread_cond_wait(&task_queue_cond[index], &task_queue_mutex[index]);
			pthread_mutex_unlock(&task_queue_mutex[index]);
		}
	}
}
void sched_pbs(float currentTime, int tid, int remainingTime, int tprio) {
	if (remainingTime == 0) {
		//fprintf(stderr, "remainingTime = 0\n");
		if (!headbechanged){
			listhead = listhead->nextnode;
		}
		else{
			headbechanged = 0;
		}
		pthread_mutex_unlock(&queue_mutex);
		// task complete, pick new task from queue
		/*
		if(listhead == NULL){
		fprintf(stderr, "listhead = 0\n");
		}
		*/
		if (listhead != NULL) {
			__current_tid = listhead->ID;
			// signals the next task if any
			//fprintf(stderr, "Signaling: %d\n", listhead->ID);
			pthread_cond_signal(&task_queue_cond[listhead->index]);
		}
	}
	else if (headbechanged){
		//fprintf(stderr, "headbechanged = %d\n", listhead->ID);
		headbechanged = 0;
		__current_tid = listhead->ID;
		//fprintf(stderr, "Signaling: %d\n", listhead->ID);
		pthread_mutex_unlock(&queue_mutex);
		pthread_cond_signal(&task_queue_cond[listhead->index]);

		//fprintf(stderr, "enqueuing = %d\n", tid);
		// enqueue new task and block it
		pthread_mutex_lock(&queue_mutex);
		int index = queue_back++;
		listhead = enqueuenode(listhead, tid, index, tprio, currentTime);
		pthread_mutex_unlock(&queue_mutex);
		pthread_cond_init(&task_queue_cond[index], NULL);
		pthread_mutex_init(&task_queue_mutex[index], NULL);

		while (__current_tid != tid) {
			pthread_mutex_lock(&task_queue_mutex[index]);
			pthread_cond_wait(&task_queue_cond[index], &task_queue_mutex[index]);
			pthread_mutex_unlock(&task_queue_mutex[index]);
		}
	}
	else if (tid == __current_tid){
		listhead->value = tprio;
		listhead->subvalue = currentTime;
		pthread_mutex_unlock(&queue_mutex);
	}
	else{
		if (listhead == NULL){
			__current_tid = tid;
		}
		//fprintf(stderr, "enqueuing = %d\n", tid);
		// enqueue new task and block it
		int index = queue_back++;
		listhead = enqueuenode(listhead, tid, index, tprio, currentTime);
		pthread_mutex_unlock(&queue_mutex);
		pthread_cond_init(&task_queue_cond[index], NULL);
		pthread_mutex_init(&task_queue_mutex[index], NULL);

		while (__current_tid != tid) {
			pthread_mutex_lock(&task_queue_mutex[index]);
			pthread_cond_wait(&task_queue_cond[index], &task_queue_mutex[index]);
			pthread_mutex_unlock(&task_queue_mutex[index]);
		}
	}
}
void sched_mlfq(float currentTime, int tid, int remainingTime) {
	//check if execute or block
	if (remainingTime == 0){
		int tlevel = level[tid];
		if (!insertlower){
			multihead[tlevel - 1] = multihead[tlevel - 1]->nextnode;
		}
		else{
			insertlower = 0;
		}
		pthread_mutex_unlock(&queue_mutex);

		while (multihead[tlevel - 1] == NULL && tlevel < 5){
			tlevel++;
			//fprintf(stderr, "tlevel = %d\n", tlevel);
		}
		if (multihead[tlevel - 1] != NULL) {
			__current_tid = multihead[tlevel - 1]->ID;
			// signals the next task if any
			//fprintf(stderr, "Remaining0 and Signaling: %d\n", __current_tid);
			pthread_cond_signal(&task_queue_cond[multihead[tlevel - 1]->index]);
			pthread_mutex_lock(&queue_mutex);
			//multihead[tlevel - 1] = multihead[tlevel - 1]->nextnode;
			pthread_mutex_unlock(&queue_mutex);
		}
	}
	else if (insertlower){
		//fprintf(stderr, "insert lower\t");
		insertlower = 0;
		__current_tid = multihead[0]->ID;
		//fprintf(stderr, "Signaling: %d\n", multihead[0]->ID);
		pthread_mutex_unlock(&queue_mutex);
		pthread_cond_signal(&task_queue_cond[multihead[0]->index]);

		
		// enqueue new task and block it
		if(multihead[level[tid] - 1] != NULL){
			multihead[level[tid] - 1] = multihead[level[tid] - 1]->nextnode;
		}
		if (level[tid] == 0){
			level[tid] = 1;
		}
		int tlevel = level[tid];
		tcount[tid]++;
		int index = queue_back++;
		//fprintf(stderr, "enqueuing = %d, index = %d\n", tid, index);
		pthread_mutex_lock(&queue_mutex);
		multihead[tlevel - 1] = enqueuemulti(multihead[tlevel - 1], tid, index, 0, 0);
		pthread_mutex_unlock(&queue_mutex);
		pthread_cond_init(&task_queue_cond[index], NULL);
		pthread_mutex_init(&task_queue_mutex[index], NULL);

		while (__current_tid != tid) {
			pthread_mutex_lock(&task_queue_mutex[index]);
			pthread_cond_wait(&task_queue_cond[index], &task_queue_mutex[index]);
			pthread_mutex_unlock(&task_queue_mutex[index]);
		}
	}//check __current_tid
	else if (tid == __current_tid){
		int tlevel = level[tid];
		if (tcount[tid] >= (5 * tlevel)){
			// signal others
			pthread_mutex_unlock(&queue_mutex);
			multihead[tlevel - 1] = multihead[tlevel - 1]->nextnode;
			while (multihead[tlevel - 1] == NULL && tlevel < 5){
				tlevel++;
			}
			if (multihead[tlevel - 1] != NULL) {
				__current_tid = multihead[tlevel - 1]->ID;
				// signals the next task if any
				//fprintf(stderr, "Overflow and Signaling: %d, __current_tid= %d, tlevel = %d, index = %d\n", __current_tid, __current_tid, tlevel, multihead[tlevel - 1]->index);
				pthread_cond_signal(&task_queue_cond[multihead[tlevel - 1]->index]);
				
				// block iteself to the end of next level
				if (level[tid] < 5){
					level[tid]++;
				}
				tlevel = level[tid];
				tcount[tid] = 0;
				node* thead = multihead[tlevel - 1];
				tcount[tid]++;
				int index = queue_back++;
				//fprintf(stderr, "sending %d to the level %d, index = %d\n", tid, tlevel, index);
				pthread_mutex_lock(&queue_mutex);
				multihead[tlevel - 1] = enqueuemulti(thead, tid, index, 0, 0);
				pthread_mutex_unlock(&queue_mutex);
				pthread_cond_init(&task_queue_cond[index], NULL);
				pthread_mutex_init(&task_queue_mutex[index], NULL);

				while (__current_tid != tid) {
					pthread_mutex_lock(&task_queue_mutex[index]);
					pthread_cond_wait(&task_queue_cond[index], &task_queue_mutex[index]);
					pthread_mutex_unlock(&task_queue_mutex[index]);
				}
			}
			else{
				// no others to signal, keep going
				// if (multihead[level[tid]-1] != NULL){
					// multihead[level[tid]-1]= multihead[level[tid]-1]->nextnode;
				// }
				if (level[tid] < 5){
					level[tid]++;
				}
				tlevel = level[tid];
				tcount[tid] = 0;
				node* thead = multihead[tlevel - 1];
				tcount[tid]++;
				
				pthread_mutex_lock(&queue_mutex);
				int index = queue_back++;
				//fprintf(stderr, "sending %d to the level %d, index = %d\n", tid, tlevel, index);
				multihead[tlevel - 1] = enqueuemulti(thead, tid, index, 0, 0);
				pthread_mutex_unlock(&queue_mutex);
				return;
			}
		}
		else{
			tcount[tid]++;
			pthread_mutex_unlock(&queue_mutex);
		}
	}//block as following
	else if (level[tid] == 0){
		level[tid] = 1;
		int tlevel = 1;
		tcount[tid]++;
		// check whether the whole queues are all empty
		int testlevel = 1;
		while (multihead[testlevel - 1] == NULL && testlevel < 5){
			testlevel++;
			//fprintf(stderr, "testlevel = %d\n", testlevel);
		}
		if (multihead[testlevel - 1] == NULL){
			mlsqfirsttime = 1;
			//fprintf(stderr, "empty queues detected\n");
		}
		// Put into first level
		if (multihead[0] == NULL && !mlsqfirsttime){
			//fprintf(stderr, "insert lower, ID = %d\n", tid);
			insertlower = 1;
		}
		int index = queue_back++;
		multihead[0] = enqueuemulti(multihead[0], tid, index, 0, 0);
		pthread_mutex_unlock(&queue_mutex);
		pthread_cond_init(&task_queue_cond[index], NULL);
		pthread_mutex_init(&task_queue_mutex[index], NULL);
		
		if (mlsqfirsttime){
			__current_tid = tid;
			mlsqfirsttime = 0;
		}

		while (__current_tid != tid) {
			pthread_mutex_lock(&task_queue_mutex[index]);
			pthread_cond_wait(&task_queue_cond[index], &task_queue_mutex[index]);
			pthread_mutex_unlock(&task_queue_mutex[index]);
		}
	}
	else if (level[tid] <= 4){// level 1 to 4
		// schedule
		int tlevel = level[tid];
		node* thead = multihead[tlevel - 1];
		tcount[tid]++;
		int index = queue_back++;
		multihead[tlevel - 1] = enqueuemulti(thead, tid, index, 0, 0);
		pthread_mutex_unlock(&queue_mutex);
		pthread_cond_init(&task_queue_cond[index], NULL);
		pthread_mutex_init(&task_queue_mutex[index], NULL);

		while (__current_tid != tid) {
			pthread_mutex_lock(&task_queue_mutex[index]);
			pthread_cond_wait(&task_queue_cond[index], &task_queue_mutex[index]);
			pthread_mutex_unlock(&task_queue_mutex[index]);
		}

	}//insert to level 5
	else{

		node* thead = multihead[4];
		tcount[tid]++;
		int index = queue_back++;
		multihead[4] = enqueuemulti(thead, tid, index, 0, 0);
		pthread_mutex_unlock(&queue_mutex);
		pthread_cond_init(&task_queue_cond[index], NULL);
		pthread_mutex_init(&task_queue_mutex[index], NULL);

		while (__current_tid != tid) {
			pthread_mutex_lock(&task_queue_mutex[index]);
			pthread_cond_wait(&task_queue_cond[index], &task_queue_mutex[index]);
			pthread_mutex_unlock(&task_queue_mutex[index]);
		}

	}
}

// multi enqueue
node* enqueuemulti(node*head, int ID, int index, int value, float subvalue){
	//insert from head
	if (head == NULL){
		//fprintf(stderr, "condition 1, ID = %d\n", ID);
		node* newhead = malloc(sizeof(node));
		newhead->ID = ID;
		newhead->index = index;
		newhead->nextnode = NULL;
		newhead->value = value;
		newhead->subvalue = subvalue;
		//headbechanged = 1;
		return newhead;
	}
	// find the tail
	node* ptr = head;
	node* nptr = head->nextnode;
	while (nptr != NULL){
		ptr = nptr;
		nptr = ptr->nextnode;
	}

	//insert to the tail
	//fprintf(stderr, "enqueuing %d\n", ID);
	node* newnode = malloc(sizeof(node));
	newnode->nextnode = NULL;
	newnode->ID = ID;
	newnode->index = index;
	newnode->value = value;
	newnode->subvalue = subvalue;
	ptr->nextnode = newnode;
	return head;
}

// srtf enqueue
node* enqueuenode(node*head, int ID, int index, int value, float subvalue){

	//insert from head
	if (head == NULL){
		//fprintf(stderr, "condition 1");
		node* newhead = malloc(sizeof(node));
		newhead->ID = ID;
		newhead->index = index;
		newhead->nextnode = NULL;
		newhead->value = value;
		newhead->subvalue = subvalue;
		//headbechanged = 1;
		return newhead;
	}
	else if (value < head->value){
		//fprintf(stderr, "condition 2");
		node* newhead = malloc(sizeof(node));
		newhead->ID = ID;
		newhead->index = index;
		newhead->nextnode = head->nextnode;
		newhead->value = value;
		newhead->subvalue = subvalue;
		headbechanged = 1;
		return newhead;
	}
	else if (value == head->value && subvalue < head->subvalue){
		//fprintf(stderr, "condition 3");
		node* newhead = malloc(sizeof(node));
		newhead->ID = ID;
		newhead->index = index;
		newhead->nextnode = head->nextnode;
		newhead->value = value;
		newhead->subvalue = subvalue;
		headbechanged = 1;
		return newhead;
	}

	// insert in the middle
	node* ptr = head;
	node* nptr = head->nextnode;
	while (nptr != NULL){
		if (value < nptr->value){
			node* newnode = malloc(sizeof(node));
			newnode->ID = ID;
			newnode->index = index;
			newnode->nextnode = nptr;
			newnode->value = value;
			newnode->subvalue = subvalue;
			ptr->nextnode = newnode;
			return head;
		}
		else if (value == nptr->value && subvalue < nptr->subvalue){
			node* newnode = malloc(sizeof(node));
			newnode->ID = ID;
			newnode->index = index;
			newnode->nextnode = nptr;
			newnode->value = value;
			newnode->subvalue = subvalue;
			ptr->nextnode = newnode;
			return head;
		}
		ptr = nptr;
		nptr = ptr->nextnode;
	}

	//insert to the tail
	node* newnode = malloc(sizeof(node));
	newnode->nextnode = NULL;
	newnode->ID = ID;
	newnode->index = index;
	newnode->value = value;
	newnode->subvalue = subvalue;
	ptr->nextnode = newnode;
	return head;
}
