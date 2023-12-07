#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

// Define the number of tasks and the RR quantum time.
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
	double totalTime;
	double waitTime;
	double turnaroundTime;
	double startTime;
	double endTime;
}Task;

// Define the task list and the index of the list.
Task *Tasklist;
int Listindex=0;

void Handle_file(char *filename);
void Task_initialize(Task* T, int taskID, int burstTime[], double dmax, double ro, int priority);
int Parameterize_p(Task* T);
void Parameterize_r(Task* T, int tp);
void Parameterize_b(Task* T, int tp);
void Parameterize(Task* T);
void Calculate_delay(Task* T);
void Calculate_R(Task* T);
void Calculate_quantumTime(Task* T);
int cmp(const void *T1, const void *T2);
void sort_by_priority(Task *T);
void RR_scheduling(Task* Tasklist);
void WRR_scheduling(Task* Tasklist);
void TaskRun(Task *T, double *clock, double *waitTime_increment);
void WRR_scheduling_edited();

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
    //WRR_scheduling_edited();
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
		
		double dmax, ro;
		int priority;
		fscanf(fp, "%lf %lf %d", &dmax, &ro, &priority);
		
		Task_initialize(&Tasklist[i], taskID, burstTime, dmax, ro, priority);
	}
	
	fclose(fp);
}

void Task_initialize(Task* T, int taskID, int burstTime[], double dmax, double ro, int priority)
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
	T->totalTime = 30.00;
	T->isFinish = false;
	T->waitTime = 0.00;
	T->turnaroundTime = 0.00;
	T->startTime = 0.00;
	T->endTime = 0.00;
	
	Parameterize(T);
	Calculate_quantumTime(T);
	printf("Task%d\n", T->taskID);
	printf("p = %lf\n", T->p);
	printf("r = %lf\n", T->r);
	printf("b = %lf\n", T->b);
	printf("dmax = %d\n", T->dmax);
	printf("dq = %lf\n", T->dq);
	printf("s = %lf\n", T->s);
	printf("R = %lf\n", T->R);
	printf("QuntumTime = %lf\n", T->quantumTime);
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

/*
	Written by Jacky Huo
*/

void sort_by_priority(Task *T) {
	qsort(&T[1], Listindex, sizeof(T[1]), cmp);
	
	for(int i=1; i<=Listindex; i++)
	{
		printf("Task%d, priority:%d\n", T[i].taskID, T[i].priority);
	}
}

void RR_scheduling(Task *T) //static quantum time(initialized 4)
{
	printf("RR\n");
	printf("number of tasks:%d\n", Listindex);
	int start = 1; //task id start from 1
	sort_by_priority(Tasklist);
	
	while(1)
	{	
		for(int i=start; i<=Listindex; i++)
		{
			if(T[i].totalTime > RR_QUANTUM)
			{
				T[i].totalTime -= RR_QUANTUM;
				
				if(T[i].startTime > T[i].endTime) //skip the first round
					T[i].waitTime += (T[i].startTime - T[i].endTime); 
				T[i].endTime = T[i].startTime + RR_QUANTUM; 
				T[i+1].startTime = T[i].endTime; //start time of next task is the end time of last task
				if(i==Listindex) //new turn
					T[start].startTime = T[i].endTime;
			}
			
			else
			{
				if(T[i].startTime > T[i].endTime) //skip the first round
					T[i].waitTime += (T[i].startTime - T[i].endTime); 
				T[i].endTime = T[i].startTime + RR_QUANTUM; 
				T[i+1].startTime = T[i].endTime; //start time of next task is the end time of last task
				if(i==Listindex) //new turn
					T[start].startTime = T[i].endTime;
				T[i].turnaroundTime = T[i].endTime;
				
				Tasklist[i].totalTime = 0;
				Tasklist[i].isFinish = true;
				start = i+1; 
				/* if the task is finished, start from the next(supposed all tasks have same total time).
				   Since in traditional Round Robin scheduling, all tasks are allocated the same quantum time, the ealier tasks are executed, the earlier they will finish.
				   Normally, the tasks will be finished by order since we have sorted them with priority, that's why when a task is finished we start from the next.
				 */
			}
			
			printf("Tasks %d is executed, priority: %d, quantum time: %f, time left: %f, start time:%f, end time:%f, wait time:%f\n", T[i].taskID, T[i].priority, T[i].quantumTime, T[i].totalTime, T[i].startTime, T[i].endTime, T[i].waitTime);
		}
		
		printf("\n");
		
		if(start == Listindex+1) //all tasks are finished
			break;
	}
	
	for(int i=1; i<=Listindex; i++)
		printf("Tasks %d with priority %d: waiting time is %f, turn around time is %f\n", T[i].taskID, T[i].priority, T[i].waitTime, T[i].turnaroundTime);
}

void WRR_scheduling(Task *T) //Use the same executing order as RR to precisely compare the impact of dynamic quantum time
{
	printf("WRR\n");
	printf("number of tasks:%d\n", Listindex);
	int start = 1;
	sort_by_priority(T);
	
	while(1)
	{	
		for(int i=start; i<=Listindex; i++)
		{
			if(T[i].totalTime > T[i].quantumTime)
			{
				T[i].totalTime -= T[i].quantumTime;
				
				if(T[i].startTime > T[i].endTime) //skip the first round
					T[i].waitTime += (T[i].startTime - T[i].endTime); 
				T[i].endTime = T[i].startTime + T[i].quantumTime; 
				T[i+1].startTime = T[i].endTime; //start time of next task is the end time of last task
				if(i==Listindex) //new turn
					T[start].startTime = T[i].endTime;
			}
			
			else
			{
				if(T[i].startTime > T[i].endTime) //skip the first round
					T[i].waitTime += (T[i].startTime - T[i].endTime); 
				T[i].endTime = T[i].startTime + T[i].quantumTime; 
				T[i+1].startTime = T[i].endTime; //start time of next task is the end time of last task
				if(i==Listindex) //new turn
					T[start].startTime = T[i].endTime;
				T[i].turnaroundTime = T[i].endTime;
				
				Tasklist[i].totalTime = 0;
				Tasklist[i].isFinish = true;
				start = i+1; 
				/* if the task is finished, strat from the next(supposed all tasks have same total time).
				   Since In Weighted RR, the tasks with higher priority will be allocated more quantum time as expected, so they will be finished earlier then lower priority task. 
				   Normally, the tasks will be finished by order since we have sorted them with priority, that's why when a task is finished we start from the next.
				*/
			}
			
			printf("Tasks %d is executed, priority: %d, quantum time: %f, time left: %f, start time:%f, end time:%f, wait time:%f\n", T[i].taskID, T[i].priority, T[i].quantumTime, T[i].totalTime, T[i].startTime, T[i].endTime, T[i].waitTime);
			
		}
		printf("\n");
		
		if(start == Listindex+1) //all tasks are finished
			break;
	}
	
	for(int i=1; i<=Listindex; i++)
		printf("Tasks %d with priority %d: waiting time is %f, turn around time is %f\n", T[i].taskID, T[i].priority, T[i].waitTime, T[i].turnaroundTime);
}


/*
	Edited by Hoyue. Using different style to complete the algorithm.
*/
void WRR_scheduling_edited()
{
	printf("WRR EDITED\n");
	sort_by_priority(Tasklist);

	// Initialize the clock.
	double clock = 0.00;

	// Start the scheduling.
	while(true){
		// Check if all tasks are finished. This is a counter, if all tasks are finished, it will be equal to Listindex.
		int isAllFinish = 0;

		for (int i=1;i<=Listindex;i++)
		{
			// Set the waiting time increment to 0.00.
			double waitTime = 0.00;
			TaskRun(&Tasklist[i], &clock, &waitTime);

			// Update the waiting time of other tasks.
			for (int j=1;j<=Listindex;j++)
			{
				if(i != j && Tasklist[j].isFinish == false)
					Tasklist[j].waitTime += waitTime;
			}

			// Check if the task is finished.
			if (Tasklist[i].isFinish == true)
			{
				isAllFinish++;
			}

			printf("Task%d is executed, priority: %d, quantum time: %lf, time left: %lf, wait time:%lf, present time:%lf\n", Tasklist[i].taskID, Tasklist[i].priority, Tasklist[i].quantumTime, Tasklist[i].totalTime, Tasklist[i].waitTime, clock);
		}
		printf("\n");

		if (isAllFinish == Listindex)
		{
			break;
		}
	}

	printf("All tasks are finished\n");
	for(int i=1; i<=Listindex; i++)
		printf("Tasks %d with priority %d: waiting time is %f, turn around time is %f\n", Tasklist[i].taskID, Tasklist[i].priority, Tasklist[i].waitTime, Tasklist[i].turnaroundTime);
}

void TaskRun(Task *T, double *clock, double *waitTime_increment)
{
	// Check if the task is finished.
	if(T->isFinish)
	{
		return;
	}

	// Executed and calculate the cost.
	if(T->totalTime > T->quantumTime)    //task is not finished yet
	{
		T->totalTime -= T->quantumTime;
		*clock += T->quantumTime;    //clock adds the quantum time
		*waitTime_increment = T->quantumTime;    //wait time increment is the quantum time
	}
	else    //task will finish
	{
		T->isFinish = true;
		*clock += T->totalTime;    //clock adds the time left
		T->turnaroundTime = *clock;    //turn around time is the present time
		*waitTime_increment = T->totalTime;    //wait time increment is the time left
	}
	
}

