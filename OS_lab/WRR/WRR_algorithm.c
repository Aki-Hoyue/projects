// The new version supports different Totaltime and Arrivaltime, see WRR_algorithm_opt.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define TASK_ROUND 3
#define RR_QUANTUM 4

typedef struct task{
	int taskID;
	int burstTime[TASK_ROUND + 1];
	int totalBurstTime[TASK_ROUND + 1];
	double averageBurstTime[TASK_ROUND + 1];
	double p;
	double r;
	double b;
	int dmax;
	double dq;
	double s;
	int ro;
	int priority;
	double R;
	double quantumTime;
	bool isFinish;
	double total_time;
	double start_time; //record the time when a task start using the quantum time
	double end_time; //record the time when a task leave the queue
	double wait; //record the waiting time of tasks
	double turnaround; //record the turn around time of tasks
}Task;

// Define the task list and the index of the list.
Task *Tasklist;
int Listindex=0;

void Handle_file(char *filename);
void Task_initialize(Task* T, int taskID, int burstTime[], int dmax, int ro, int priority);
int Parameterize_p(Task* T);
void Parameterize_r(Task* T, int tp);
void Parameterize_b(Task* T, int tp);
void Parameterize(Task* T);
void Calculate_delay(Task* T);
void Calculate_R(Task* T);
void Calculate_quantumTime(Task* T);
int cmp(const void *T1, const void *T2);
void sort_by_priority(Task *T, int task_num);
void RR_scheduling(Task* Tasklist);
void WRR_scheduling(Task* Tasklist);

int main(int argc, char *argv[])
{
    // Allow user to offer a file or use the default file.
    char *filename;
    if (argc != 2) {
        filename = "task.txt";
    } else {
        filename = argv[1];
    }

    // Handle the file.
    Handle_file(filename);
	RR_scheduling(Tasklist);
	Handle_file(filename);
	WRR_scheduling(Tasklist);
    
    return 0;
}

/*
	Designed by Hoyue
*/

void Handle_file(char *filename)
{
	FILE *fp;
	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("File open error.\n");
		exit(1);
	}
	
	int n;    // the number of tasks
	fscanf(fp, "%d", &n);
	Tasklist = (Task *)malloc(sizeof(Task) * (n+1));
	Listindex = n;
	
	for(int i=1;i<=n;i++)
	{
		int taskID;
		fscanf(fp, "%d", &taskID);
		
		int burstTime[TASK_ROUND + 1];
		for(int j=1;j<=TASK_ROUND;j++)
		{
			fscanf(fp, "%d", &burstTime[j]);
		}
		
		int dmax, ro, priority;
		fscanf(fp, "%d %d %d", &dmax, &ro, &priority);
		
		Task_initialize(&Tasklist[i], taskID, burstTime, dmax, ro, priority);
	}
	
	fclose(fp);
}

void Task_initialize(Task* T, int taskID, int burstTime[], int dmax, int ro, int priority)
{
	T->taskID = taskID;
	
	memset(T->burstTime, 0, sizeof(int) * (TASK_ROUND + 1));
	memset(T->totalBurstTime, 0, sizeof(int) * (TASK_ROUND + 1));
	memset(T->averageBurstTime, 0.0, sizeof(double) * (TASK_ROUND + 1));
	
	for(int i=1;i<=TASK_ROUND;i++)
	{
		T->burstTime[i] = burstTime[i];
		T->totalBurstTime[i] = T->totalBurstTime[i-1] + burstTime[i];
		T->averageBurstTime[i] = (double)(T->totalBurstTime[i]) / (double)(i * 10);
	}
	T->dmax = dmax;
	T->priority = priority;
	T->ro = ro;
	T->p = 0;
	T->r = 0;
	T->b = 0;
	T->dq = 0;
	T->s = 0;
	T->R = 0;
	T->quantumTime = 0;
	T->total_time = 30.00;
	T->isFinish = false;
	
	Parameterize(T);
	Calculate_quantumTime(T);
	
	printf("Task %d: quntumTime = %lf\n", T->taskID, T->quantumTime);
}

int Parameterize_p(Task* T)
{
	double Max = 0.0;
	int tp = 0;
	for(int i=1;i<=TASK_ROUND;i++)
	{
		if(T->averageBurstTime[i] > Max)
		{
			Max = T->averageBurstTime[i];
			tp = i;
		}
	}
	T->p = Max;
	
	return tp;
}

void Parameterize_r(Task* T, int tp)
{
	double sum = 0;
	for(int i=tp+1; i<=TASK_ROUND; i++)
	{
		sum += T->burstTime[i];
	}
	T->r = sum/((TASK_ROUND-tp)*10);
}

void Parameterize_b(Task* T, int tp)
{
	T->b = (double)(T->p - T->r) * (double)(tp * 10);
}

void Parameterize(Task* T)
{
	int tp = Parameterize_p(T);
	Parameterize_r(T, tp);
	Parameterize_b(T, tp);
}

void Calculate_delay(Task* T)
{
	double sp;
	switch (T->priority)
{
case 1:
	sp = 0;
	break;
	
case 2:
	sp = 0.25;
	break;
	
case 3:
	sp = 0.5;
	break;
}
	
	T->s = (double)(T->dmax + T->ro) * sp;
	T->dq = (double)(T->dmax) - T->s;
}

void Calculate_R(Task* T)
{
	
	T->R = T->p / (1 + T->dq * (T->p - T->r) / T->b);
}

void Calculate_quantumTime(Task* T)
{
	Calculate_delay(T);
	Calculate_R(T);
	T->quantumTime = T->R * T->ro;
}

/*
	Written by Jacky Huo
*/


int cmp(const void *T1, const void *T2) {
	Task *task1 = (Task *)T1;
	Task *task2 = (Task *)T2;
	
	if (task1->priority == task2->priority) {
		if (task1->quantumTime < task2->quantumTime) {
			return 1;
		} else if (task1->quantumTime > task2->quantumTime) {
			return -1;
		}
		else return 0;
		
	} else {
		if (task1->priority < task2->priority) {
			return 1;
		} else if (task1->priority > task2->priority) {
			return -1;
		}
		else return 0;
	}
}

void sort_by_priority(Task *T, int task_num) //sort tasks with priority, higher priority first(using qsort)
{
	qsort(&T[1], task_num, sizeof(T[1]), cmp);
	
	for(int i=1; i<=task_num; i++)
	{
		printf("Task%d, priority:%d\n", T[i].taskID, T[i].priority);
	}
}

void RR_scheduling(Task *T) //static quantum time(initialized 4)
{
	printf("RR\n");
	printf("number of tasks:%d\n", Listindex);
	int start = 1; //task id start from 1
	sort_by_priority(Tasklist, Listindex);
	
	while(1)
	{	
		for(int i=start; i<=Listindex; i++)
		{
			if(T[i].total_time > RR_QUANTUM)
			{
				T[i].total_time -= RR_QUANTUM;
				
				if(T[i].start_time > T[i].end_time) //skip the first round
					T[i].wait += (T[i].start_time - T[i].end_time); 
				T[i].end_time = T[i].start_time + RR_QUANTUM; 
				T[i+1].start_time = T[i].end_time; //start time of next task is the end time of last task
				if(i==Listindex) //new turn
					T[start].start_time = T[i].end_time;
			}
			
			else
			{
				if(T[i].start_time > T[i].end_time) //skip the first round
					T[i].wait += (T[i].start_time - T[i].end_time); 
				T[i].end_time = T[i].start_time + T[i].total_time; 
				T[i+1].start_time = T[i].end_time; //start time of next task is the end time of last task
				if(i==Listindex) //new turn
					T[start].start_time = T[i].end_time;
				T[i].turnaround = T[i].end_time;
				
				Tasklist[i].total_time = 0;
				Tasklist[i].isFinish = true;
				start = i+1; 
				/* if the task is finished, start from the next(supposed all tasks have same total time).
				   Since in traditional Round Robin scheduling, all tasks are allocated the same quantum time, the ealier tasks are executed, the earlier they will finish.
				   Normally, the tasks will be finished by order since we have sorted them with priority, that's why when a task is finished we start from the next.
				 */
			}
			
			printf("Tasks %d is executed, priority: %d, quantum time: %f, time left: %f, start time:%f, end time:%f, wait time:%f\n", T[i].taskID, T[i].priority, T[i].quantumTime, T[i].total_time, T[i].start_time, T[i].end_time, T[i].wait);
		}
		
		printf("\n");
		
		if(start == Listindex+1) //all tasks are finished
			break;
	}
	
	for(int i=1; i<=Listindex; i++)
		printf("Tasks %d with priority %d: waiting time is %f, turn around time is %f\n", T[i].taskID, T[i].priority, T[i].wait, T[i].turnaround);
}

void WRR_scheduling(Task *T) //Use the same executing order as RR to precisely compare the impact of dynamic quantum time
{
	printf("WRR\n");
	printf("number of tasks:%d\n", Listindex);
	int start = 1;
	sort_by_priority(T, Listindex);
	
	while(1)
	{	
		for(int i=start; i<=Listindex; i++)
		{
			if(T[i].total_time > T[i].quantumTime)
			{
				T[i].total_time -= T[i].quantumTime;
				
				if(T[i].start_time > T[i].end_time) //skip the first round
					T[i].wait += (T[i].start_time - T[i].end_time); 
				T[i].end_time = T[i].start_time + T[i].quantumTime; 
				T[i+1].start_time = T[i].end_time; //start time of next task is the end time of last task
				if(i==Listindex) //new turn
					T[start].start_time = T[i].end_time;
			}
			
			else
			{
				if(T[i].start_time > T[i].end_time) //skip the first round
					T[i].wait += (T[i].start_time - T[i].end_time); 
				T[i].end_time = T[i].start_time + T[i].total_time; 
				T[i+1].start_time = T[i].end_time; //start time of next task is the end time of last task
				if(i==Listindex) //new turn
					T[start].start_time = T[i].end_time;
				T[i].turnaround = T[i].end_time;
				
				Tasklist[i].total_time = 0;
				Tasklist[i].isFinish = true;
				start = i+1; 
				/* if the task is finished, strat from the next(supposed all tasks have same total time).
				   Since In Weighted RR, the tasks with higher priority will be allocated more quantum time as expected, so they will be finished earlier then lower priority task. 
				   Normally, the tasks will be finished by order since we have sorted them with priority, that's why when a task is finished we start from the next.
				*/
			}
			
			printf("Tasks %d is executed, priority: %d, quantum time: %f, time left: %f, start time:%f, end time:%f, wait time:%f\n", T[i].taskID, T[i].priority, T[i].quantumTime, T[i].total_time, T[i].start_time, T[i].end_time, T[i].wait);
			
		}
		printf("\n");
		
		if(start == Listindex+1) //all tasks are finished
			break;
	}
	
	for(int i=1; i<=Listindex; i++)
		printf("Tasks %d with priority %d: waiting time is %f, turn around time is %f\n", T[i].taskID, T[i].priority, T[i].wait, T[i].turnaround);
}


