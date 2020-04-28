#define _GNU_SOURCE
#include "process.h"
#include "scheduler.h"
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sched.h>

/* Last context switch time for RR scheduling */
static int t_last;

/* Current unit time */
static int ntime;

/* Index of running process. -1 if no process running */
static int running;

/* Number of finish Process */
static int finish_cnt;

/* Sort processes by ready time */
int cmp(const void *a, const void *b) {
	return ((struct process *)a)->t_ready - ((struct process *)b)->t_ready;
}

/* Return index of next process  */
int next_process(struct process *proc, int amount, int policy)
{
	/* Non-preemptive */
	if (running != -1 && (policy == SJF || policy == FIFO))
		return running;

	int ret = -1;

	if (policy == PSJF || policy ==  SJF) {
		for (int i = 0; i < amount; i++) {
			if (proc[i].pid == -1 || proc[i].t_exec == 0)
				continue;
			if (ret == -1 || proc[i].t_exec < proc[ret].t_exec)
				ret = i;
		}
	}

	else if (policy == FIFO) {
		for(int i = 0; i < amount; i++) {
			if(proc[i].pid == -1 || proc[i].t_exec == 0)
				continue;
			if(ret == -1 || proc[i].t_ready < proc[ret].t_ready)
				ret = i;
		}
        }

	else if (policy == RR) {
		if (running == -1) {
			for (int i = 0; i < amount; i++) {
				if (proc[i].pid != -1 && proc[i].t_exec > 0){
					ret = i;
					break;
				}
			}
		}
		else if ((ntime - t_last) % 500 == 0)  {
			ret = (running + 1) % amount;
			while (proc[ret].pid == -1 || proc[ret].t_exec == 0)
				ret = (ret + 1) % amount;
		}
		else
			ret = running;
	}

	return ret;
}

int scheduling(struct process *proc, int amount, int policy)
{
	qsort(proc, amount, sizeof(struct process), cmp);

	/* Initial pid = -1 imply not ready */
	for (int i = 0; i < amount; i++)
		proc[i].pid = -1;

	/* Set single core prevent from preemption */
	assign_cpu(getpid(), PARENT_CPU);
	
	/* Set high priority to scheduler */
	wakeup(getpid());
	
	/* Initial scheduler */
	ntime = 0;
	running = -1;
	finish_cnt = 0;
	
	while(1) {
		//fprintf(stderr, "Current time: %d\n", ntime);

		/* Check if running process finish */
		if (running != -1 && proc[running].t_exec == 0) {
		
#ifdef DEBUG
			fprintf(stderr, "%s finish at time %d.\n", proc[running].name, ntime);
#endif
			//kill(running, SIGKILL);
			waitpid(proc[running].pid, NULL, 0);
			printf("%s %d\n", proc[running].name, proc[running].pid);
			running = -1;
			finish_cnt++;

			/* All process finish */
			if (finish_cnt == amount)
				break;
		}

		/* Check if process ready and execute */
		for (int i = 0; i < amount; i++) {
			if (proc[i].t_ready == ntime) {
				proc[i].pid = exec(proc[i]);
				block(proc[i].pid);

			}

		}

		/* Select next running  process */
		int next = next_process(proc, amount, policy);
		if (next != -1) {
			/* Context switch */
			if (running != next) {
				wakeup(proc[next].pid);
				block(proc[running].pid);
				running = next;
				t_last = ntime;
			}
		}

		/* Run an unit of time */
		UNIT_T();
		if (running != -1)
			proc[running].t_exec--;
		ntime++;
	}

	return 0;
}
