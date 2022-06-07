// Updates to file suba posted
// Created two new functions: movetoiodev and runio
// still need to determine how long process block for I/O

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

struct process {
    char *name;
    int totalCPU;
    int completeTime;
    int givenCPU;
    int BlockedIO;
    int doingIO;
};

struct resource {
    char *name; // CPU or IO
    int busy;
    int idle;
    double utilization; // busy / (busy + idle)
    int number; // dispatches or IO time
    double throughput;
};

queue_t ready_queue;
queue_t io_queue;

queue_t stats_queue;
struct resource *sysCPU, *sysIO;

struct process* iodev; //process on IO device atm

struct process* generateProcess(char *name, int totalCPU, int completeTime, int givenCPU, int BlockedIO, int doingIO) {
    struct process* tmp = (struct process*)malloc(sizeof(struct process));
    tmp->name = name;
    tmp->totalCPU = totalCPU;
	tmp->completeTime = completeTime;
	tmp->givenCPU = givenCPU;
	tmp->BlockedIO = BlockedIO;
    tmp->doingIO = doingIO;
    return tmp;
}

void displayProcess(struct process* p) {
    printf("name\ttotalCPU\tcompleteTime\tgivenCPU\tBlockedIO\tdoingIO\n");
    printf("%s\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n", p->name, p->totalCPU, p->completeTime, p->givenCPU, p->BlockedIO, p->doingIO);
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

void displayResource(struct resource* res) {
    if(strcmp(res->name, "CPU")) {
        printf("%s:\n", res->name);
        printf("Total time spent busy: %d\n", res->busy);
        printf("Total time spent idle: %d\n", res->idle);
        printf("CPU utilization: %.2f\n", res->utilization);
        printf("Number of dispatches: %d\n", res->number);
        printf("Overall throughput: %.2f\n", res->throughput);
    }
    else if(strcmp(res->name, "IO")) {
        printf("%s:\n", res->name);
        printf("Total time spent busy: %d\n", res->busy);
        printf("Total time spent idle: %d\n", res->idle);
        printf("I/O device utilization: %.2f\n", res->utilization);
        printf("Number of times I/O was started: %d\n", res->number);
        printf("Overall throughput: %.2f\n", res->throughput);
    }
    else {
        printf("Error Resource Type");
    }
}

void movetoiodev(int remaining_runtime, struct process *ptr) {
    iodev = ptr;
    // determine runtime
    int io_runtime; 
    if (remaining_runtime == 0){
        io_runtime = 1;
    }
    else{ 
	 // generate random number
    }

    // Update process I/O runtime, total IO busy runtime
    ptr->doingIO = ptr->doingIO + io_runtime;
    sysIO->busy = sysIO->busy + io_run_time; 
    return;
}

void runio(int remaining_runtime) {
    if(io_queue->count == 0 && iodev == NULL) { //I/O list empty AND I/O device is empty
        sysIO->idle += 1;
	return;
    }
    else if(iodev == NULL){ //I/O device inactive, move first process of I/O list into I/O device
        struct process* currentProcess = io_queue->head;
	movetoiodev(remaining_runtime, currentProcess);
    }
	
    // if process has completed its I/O, move to ready queue
    queue_dequeue(io_queue, (void**)&iodev);
    queue_enqueue(ready_queue, iodev);
    iodev = NULL; // move current process of I/O device
    sysIO->idle += 1;
    return;
}

int main(void) {
    // FCFS
    (void) srandom(12345);
    ready_queue = queue_create();
    io_queue = queue_create();

    stats_queue = queue_create();

    int clock = 0;
    int busy_time = 0;
    int idle_time = 0;
    int dispatches = 0;

    //TODO: Read in processes into ready_queue

    int process_count = ready_queue->count;
    //ready_queue filled
    while (ready_queue->count > 0 && io_queue->count > 0) {
        struct process* cpu; //process on cpu atm
        int remaining_job_runtime = 0; // runtime left for process on cpu
        int requires_blocking = 0; //non-zero if process on cpu is I/O blockin, 0 otherwise

        //setup next CPU task if not running a task currently
        if (ready_queue->count > 0 && cpu == NULL) {
            cpu = ready_queue->head; //process at start of the queue
            int remaining_job_runtime = cpu->completeTime;

            double rng = (double)random() / (double)RAND_MAX; // rand between 0 and 1
            //Task blocks only if has at least 2 time units
            if (remaining_job_runtime >= 2 && rng < cpu->BlockedIO) {
                //TODO: range between 1 and runtime inclusive
                remaining_job_runtime = rand();
            }
            requires_blocking = 1;
            dispatches++;
        }

        //run task
        clock++;
        if (remaining_job_runtime > 0) {
            remaining_job_runtime--;
            busy_time++;
            //job finished if true
            if (remaining_job_runtime == 0) {
                //move to IO if blocked for IO
                if (requires_blocking != 0) {
                    queue_enqueue(io_queue, cpu);
                }
                else {
                    //otherwise finish task and gather statistics
                    //TODO for Kayla, IO Times
                    int ioblock_count = 0;
                    int io_time = 0;

                    struct process* stats = generateProcess(cpu->name, cpu->completeTime, clock, ioblock_count, io_time);
                    queue_enqueue(stats_queue, stats);
                    //move task off cpu
                    cpu = NULL;
                }
            }
        }
        else {
            idle_time++;
        }
    }
    return 0;
}
