#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct process {
	char *name;
    int totalCPU;
    int completeTime;
    int givenCPU;
    int BlockedIO;
    int doingIO;

    //for cpu & i/o usage
    double prob_block;
    int cpu_timeLeft;
    int current_io_timeLeft;
};

struct resource {
	char *name; // CPU or IO
    int busy;
    int idle;
    double utilization; // busy / (busy + idle)
    int number; // dispatches or IO time
    double throughput;
};

struct process* generateProcess(char *name, int totalCPU, int completeTime, int givenCPU, int BlockedIO, int doingIO, double prob) {
    struct process* tmp = (struct process*)malloc(sizeof(struct process));
    tmp->name = name;
    tmp->totalCPU = totalCPU;
	tmp->completeTime = completeTime;
	tmp->givenCPU = givenCPU;
	tmp->BlockedIO = BlockedIO;
    tmp->doingIO = doingIO;

    tmp->prob_block = prob;
    tmp->cpu_timeLeft = totalCPU;
    tmp->current_io_timeLeft = 0;
    return tmp;
}

void setupProcessTable() {
    printf("Processes:\n\n");
	printf("   name     CPU time  when done  cpu disp  i/o disp  i/o time\n");
}

void displayProcess(struct process* p) {
    printf("%-10s %6d     %6d    %6d    %6d    %6d\n", p->name, p->totalCPU, p->completeTime, p->givenCPU, p->BlockedIO, p->doingIO);
}

struct resource* buildResource(char *name, int busy, int idle, int number) {
    struct resource* tmp = (struct resource*)malloc(sizeof(struct resource));
    tmp->name = name;
    tmp->busy = busy;
	tmp->idle = idle;
	tmp->utilization = (double)busy / (double)(busy + idle);
	tmp->number = number;
    tmp->throughput = (double)number / (double)(busy + idle);
    return tmp;
}

void calcResourceStats(struct resource* res, int count) {
	res->utilization = (double)res->busy / (double)(res->busy + res->idle);
    res->throughput = (double)count / (double)(res->busy + res->idle);
}

void displayResource(struct resource* res) {
    if(strcmp(res->name, "CPU") == 0) {
        printf("\n%s:\n", res->name);
        printf("Total time spent busy: %d\n", res->busy);
        printf("Total time spent idle: %d\n", res->idle);
        printf("CPU utilization: %.2f\n", res->utilization);
        printf("Number of dispatches: %d\n", res->number);
        printf("Overall throughput: %.2f\n", res->throughput);
    }
    else if(strcmp(res->name, "IO") == 0) {
        printf("\nI/O device:\n");
        printf("Total time spent busy: %d\n", res->busy);
        printf("Total time spent idle: %d\n", res->idle);
        printf("I/O utilization: %.2f\n", res->utilization);
        printf("Number of dispatches: %d\n", res->number);
        printf("Overall throughput: %.2f\n", res->throughput);
    }
    else {
        printf("Error Resource Type");
    }
}