// An improved version of the WRR algorithm with different Totaltime and Arrivaltime support.
// Author: Hoyue

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// Define the round of task and the RR quantum time.
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
	int isFinish;
	double totalTime;
	double waitTime;
	double turnaroundTime;
	double arrivalTime;
	int arrivalFlag;
}Task;

// Define the task list and the index of the list.
Task *Tasklist;
int Listindex=0;

void Handle_file(char *filename);
void Task_initialize(Task *T, int taskID, double totalTime, double arrivalTime, int burstTime[], double dmax, double ro, int priority);
int Parameterize_p(Task *T);
void Parameterize_r(Task *T, int tp);
void Parameterize_b(Task *T, int tp);
void Parameterize(Task *T);
void Calculate_delay(Task *T);
void Calculate_R(Task *T);
void Calculate_quantumTime(Task *T);
int cmp(const void *T1, const void *T2);
int cmp_priority(const void *T1, const void *T2);
void Organize();
void TaskRun(Task *T, double *clock, double *waitTime_increment);
void WRR_scheduling_edited();

int main(int argc, char *argv[])
{
    // Allow user to offer a file or use the default file.
    char *filename;
    if (argc != 2) {
        filename = "task_opt.txt";
    } else {
        filename = argv[1];
    }

    // Handle the file.
    Handle_file(filename);
    WRR_scheduling_edited();
    return 0;
}

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
	
	printf("Initial situation of tasks:\n");

	for(int i=1;i<=n;i++)
	{
		int taskID;
		double totalTime, arrivalTime;
		fscanf(fp, "%d", &taskID);
		fscanf(fp, "%lf %lf", &totalTime, &arrivalTime);
		
		int burstTime[TASK_ROUND + 1];
		for(int j=1;j<=TASK_ROUND;j++)
		{
			fscanf(fp, "%d", &burstTime[j]);
		}
		
		double dmax, ro;
		int priority;
		fscanf(fp, "%lf %lf %d", &dmax, &ro, &priority);
		
		Task_initialize(&Tasklist[i], taskID, totalTime, arrivalTime, burstTime, dmax, ro, priority);
	}
	
	Organize();
	fclose(fp);
}

void Task_initialize(Task *T, int taskID, double totalTime, double arrivalTime, int burstTime[], double dmax, double ro, int priority)
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
	T->totalTime = totalTime;
	T->arrivalTime = arrivalTime;
	T->ro = ro;
	T->p = 0;
	T->r = 0;
	T->b = 0;
	T->dq = 0;
	T->s = 0;
	T->R = 0;
	T->quantumTime = 0;
	T->isFinish = 0;
	T->waitTime = 0.00;
	T->turnaroundTime = 0.00;

	// Set the arrival flag.
	if (T->arrivalTime > 0)
		// Has been arrived.
		T->arrivalFlag = 0;
	else
		// Not arrived.
		T->arrivalFlag = 1;
	
	Parameterize(T);
	Calculate_quantumTime(T);
	printf("Task%d, priority: %d, arrival time: %lf, total time: %lf, quantum time: %lf\n", T->taskID, T->priority, T->arrivalTime, T->totalTime, T->quantumTime);
}

int Parameterize_p(Task *T)
{
	// Peak rate p is the maximum average burst time.

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
	
	// Tp is the time of the peak rate.
	return tp;
}

void Parameterize_r(Task *T, int tp)
{
	// avarage rate r is the sum of the burst time from the peak rate to the end.

	double sum = 0;
	for(int i=tp+1; i<=TASK_ROUND; i++)
	{
		sum += T->burstTime[i];
	}
	T->r = sum / ((TASK_ROUND-tp) * (T->totalTime/TASK_ROUND));
}

void Parameterize_b(Task *T, int tp)
{
	// Burst size b is the difference between the peak rate and the average rate.

	T->b = (double)(T->p - T->r) * (double)(tp * (T->totalTime/TASK_ROUND));
}

void Parameterize(Task *T)
{
	int tp = Parameterize_p(T);
	Parameterize_r(T, tp);
	Parameterize_b(T, tp);
}

void Calculate_delay(Task *T)
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

void Calculate_R(Task *T)
{
	T->R = T->p / (1 + T->dq * (T->p - T->r) / T->b);
}

void Calculate_quantumTime(Task *T)
{
	Calculate_delay(T);
	Calculate_R(T);
	T->quantumTime = T->R * T->ro;
}

void Organize()
{
	// Organize the task by arrvial time or priority.

	qsort(&Tasklist[1], Listindex, sizeof(Tasklist[1]), cmp);

	printf("\nTask lists by arrival time:\n");
	for(int i=1; i<=Listindex; i++)
	{
		printf("Task%d, priority: %d, arrivalTime: %lf\n", Tasklist[i].taskID, Tasklist[i].priority, Tasklist[i].arrivalTime);
	}
}

int cmp(const void *T1, const void *T2) {
	Task *task1 = (Task *)T1;
	Task *task2 = (Task *)T2;
	
	// Sort by arrival time.
	if (task1->arrivalTime > task2->arrivalTime) {
		return 1;
	} else if (task1->arrivalTime < task2->arrivalTime) {
		return -1;
	}
	else 
		return cmp_priority(T1, T2);
}

int cmp_priority(const void *T1, const void *T2)
{
	Task *task1 = (Task *)T1;
	Task *task2 = (Task *)T2;
	
	// Sort by priority.
	if (task1->priority == task2->priority) {
		if (task1->quantumTime < task2->quantumTime) {
			return 1;
		} else if (task1->quantumTime > task2->quantumTime) {
			return -1;
		}
		else return 0;
	}
	else {
		if (task1->priority < task2->priority) {
			return 1;
		} else if (task1->priority > task2->priority) {
			return -1;
		}
		else return 0;
	}
}

// Edited by Hoyue. WRR algorithm with different arrival time.

void WRR_scheduling_edited()
{
	printf("\nWRR algorithm:\n");

	// Initialize the clock.
	double clock = Tasklist[1].arrivalTime;
	int round = 1;

	// Start the scheduling.
	while(1){
		// Check if all tasks are finished. 
		// This is a counter, if all tasks are finished, it will be equal to Listindex.
		int isAllFinish = 0, resort = 0;
		printf("Round %d\n", round++);

		for (int i=1;i<=Listindex;i++)
		{
			double backupClock = clock;

			// Check if the task is not arrived.
			if (Tasklist[i].arrivalTime > clock && Tasklist[i].arrivalFlag == 0)
				continue;

			//printf("Task%d is executing\n", Tasklist[i].taskID);
			//getchar();
			// Set the waiting time increment to 0.00.
			double waitTime = 0.00;
			TaskRun(&Tasklist[i], &clock, &waitTime);

			// Check if other task arrived.
			for (int j=i+1;j<=Listindex;j++)
			{
				if(Tasklist[j].arrivalTime <= clock && Tasklist[j].arrivalFlag == 0){
					// Task that not arrived arrive, set the arrivalFlag to 1 and resort tasklist from beginning to i.
					Tasklist[j].arrivalFlag = 1;
					resort = j;
					printf("Task%d is arrived\n", Tasklist[j].taskID);
				}
			}

			// Update the waiting time of other tasks.
			for (int j=1;j<=Listindex;j++)
			{
				if (i != j && Tasklist[j].isFinish == 0 && Tasklist[j].arrivalFlag == 1)
					Tasklist[j].waitTime += waitTime;
				
				// Update the waiting time of arrived tasks at this round.
				if (Tasklist[j].arrivalTime <= clock && Tasklist[j].arrivalTime >= backupClock)
						Tasklist[j].waitTime -= Tasklist[j].arrivalTime;
			}

			// Check if the task is finished.
			if (Tasklist[i].isFinish == 1)
			{
				isAllFinish++;
				printf("Task%d is finished\n", Tasklist[i].taskID);
			}

			// Print the task information.
			if (Tasklist[i].isFinish == 0)
				printf("Task%d is executed, priority: %d, quantum time: %lf, time left: %lf, wait time:%lf, present time:%lf\n", Tasklist[i].taskID, Tasklist[i].priority, Tasklist[i].quantumTime, Tasklist[i].totalTime, Tasklist[i].waitTime, clock);
		}
		printf("\n");

		// Resort the arrived tasklist by priority.
		if(resort > 0)
		{
			qsort(&Tasklist[1], resort, sizeof(Tasklist[1]), cmp_priority);

			printf("Resort task lists by priority\n");
			for(int i=1; i<=Listindex; i++)
			{
				printf("Task%d, priority: %d, arrival time: %lf\n", Tasklist[i].taskID, Tasklist[i].priority, Tasklist[i].arrivalTime);
			}
			printf("\n");
			resort = 0;
		}

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
	if(T->totalTime > T->quantumTime)
	{
		//task is not finished yet
		T->totalTime -= T->quantumTime;
		*clock += T->quantumTime;    //clock adds the quantum time
		*waitTime_increment = T->quantumTime;    //wait time increment is the quantum time
	}
	else
	{
		//task will finish
		T->isFinish = 1;
		*clock += T->totalTime;    //clock adds the time left
		T->turnaroundTime = *clock - T->arrivalTime;    //turn around time is the present time
		*waitTime_increment = T->totalTime;    //wait time increment is the time left
	}
	
}

