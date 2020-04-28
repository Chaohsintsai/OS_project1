#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include "process.h"
#include "scheduler.h"

int main(int argc, char* argv[])
{

	char policy_name[256];
	int policy;
	int amount;
	struct process *proc;

	scanf("%s", policy_name);
	scanf("%d", &amount);
	
	proc = (struct process *)malloc(amount * sizeof(struct process));

	for (int i = 0; i < amount; i++) {
		scanf("%s%d%d", proc[i].name,
			&proc[i].t_ready, &proc[i].t_exec);
	}

	if (strcmp(policy_name, "FIFO") == 0) {
		policy = FIFO;
	}
	else if (strcmp(policy_name, "RR") == 0) {
		policy = RR;
	}
	else if (strcmp(policy_name, "SJF") == 0) {
		policy = SJF;
	}
	else if (strcmp(policy_name, "PSJF") == 0) {
		policy = PSJF;
	}
	else {
		fprintf(stderr, "Invalid policy: %s", policy_name);
		exit(0);
	}

	scheduling(proc, amount, policy);

	exit(0);
}
