#define _GNU_SOURCE
#include "process.h"
#include <sched.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#define GET_TIME 344
#define PRINTK 343

int assign_cpu(int pid, int core)
{
	if (core > sizeof(cpu_set_t)) {
		fprintf(stderr, "Core index error.");
		return -1;
	}

	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(core, &mask);
		
	if (sched_setaffinity(pid, sizeof(mask), &mask) < 0) {
		perror("sched_setaffinity");
		exit(1);
	}

	return 0;
}

int exec(struct process proc)
{
	int pid = fork();

	if (pid < 0) {
		perror("fork");
		return -1;
	}

	if (pid == 0) {
		struct timespec start, end;
		unsigned long start_sec, start_nsec, end_sec, end_nsec;
		syscall(GET_TIME, &start);
		for (int i = 0; i < proc.t_exec; i++) {
			UNIT_T();

		}
		syscall(GET_TIME, &end);

		syscall(PRINTK, getpid(), start.tv_sec, start.tv_nsec, end.tv_sec, end.tv_nsec);
		exit(0);
	}
	
	/* Assign child to another core prevent from interupted by parant */
	assign_cpu(pid, CHILD_CPU);

	return pid;
}

int block(int pid)
{
	struct sched_param param;
	
	/* SCHED_IDLE should set priority to 0 */
	param.sched_priority = 0;

	int ret = sched_setscheduler(pid, SCHED_IDLE, &param);
	
	if (ret < 0) {
		perror("sched_setscheduler");
		return -1;
	}

	return ret;
}

int wakeup(int pid)
{
	struct sched_param param;
	
	/* SCHED_OTHER should set priority to 0 */
	param.sched_priority = 0;

	int ret = sched_setscheduler(pid, SCHED_OTHER, &param);
	
	if (ret < 0) {
		perror("sched_setscheduler");
		return -1;
	}

	return ret;
}
