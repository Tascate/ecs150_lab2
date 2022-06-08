// Updates to file suba posted
// Created two new functions: movetoiodev and runio. Also read and parsed the file
// Not sure if the way I determine how long process blocks for I/O is correct

#include <unistd.h>
#include "datamodel.h"
#include "queue.h"

const int QUANTUM_MAX = 5;
int clock = 1;
int round_robin = 0;

queue_t ready_queue;
queue_t io_queue;

struct resource *sysCPU, *sysIO; //process on CPU, sysIO
int cpu_idle = 0;
int cpu_busy = 0;

struct process* cpu; //current process running on cpu
int requires_blocking = 0; //non-zero if process on cpu is I/O blockin, 0 otherwise
int runTime_left = 0; //time to run before blocking

struct process* iodev; //process on IO device atm
int wakeup_io = 0;
int io_is_idle = 0;
int dispatches = 0;
int io_time = 0;

void prepCPU() {
    //get next process
    queue_dequeue(ready_queue, (void**)&cpu);
    runTime_left = cpu->cpu_timeLeft;

    if (round_robin) {
        runTime_left = runTime_left > QUANTUM_MAX ? QUANTUM_MAX : runTime_left;
    }

    //Task blocks only if has at least 2 time units
    if (cpu->cpu_timeLeft >= 2) {
        //Roll a number to test for blocking
        double rng = (double)random() / (double)RAND_MAX; // rand between 0 and 1
        if (rng < cpu->prob_block) {
            //generate time for running until its moved to I/O
            runTime_left = rand() % runTime_left + 1;

            //mark for blocking
            requires_blocking = 1;
        }
    }
    //increase stats
    cpu->givenCPU++;
    sysCPU->number++;
    dispatches++;
}

void runCPU() {
    if (cpu == NULL && ready_queue->count == 0) { //cpu is idle
        sysCPU->idle++;
        return;
    }
    if (cpu == NULL) { //dispatch process to CPU
        prepCPU();
    }
    if (cpu->cpu_timeLeft == 0) { //terminate process if finished
        cpu->completeTime = clock;
        //remove from cpu and display it
        displayProcess(cpu);
        cpu = NULL;
        sysCPU->idle++;

        return;
    }
    cpu->cpu_timeLeft--;
    runTime_left--;
    sysCPU->busy++;
    if (requires_blocking) {
        //block to i/o
        if (runTime_left <= 0) {
            wakeup_io = 1;
            queue_enqueue(io_queue, cpu);
            cpu = NULL;
            requires_blocking = 0;
        }
    }
    else if (round_robin) { //do round-robin quantum
        if (runTime_left <= 0) {
            queue_enqueue(ready_queue, cpu);
            cpu = NULL;
        }
    }

}

void movetoiodev(int remaining_runtime, struct process *ptr) {
    iodev = ptr;
    // determine runtime
    int io_runtime; 
    if (ptr->cpu_timeLeft == 0){
        io_runtime = 1;
    }
    else{ 
	    io_runtime = random() % 30 + 1;
    }
    ptr->current_io_timeLeft = io_runtime;
    // Update process I/O runtime, total IO busy runtime
    ptr->BlockedIO++;
    ptr->doingIO += io_runtime;
    return;
}

void runio() {
    if(io_queue->count == 0 && iodev == NULL) { //I/O list empty AND I/O device is empty
        sysIO->idle += 1;
        io_is_idle = 1;
	    return;
    }
    else if(iodev == NULL){ //I/O device inactive, move first process of I/O list into I/O device
        if(io_is_idle && wakeup_io) { //i/o does nothing if idle and cpu sent i/o work on same tick
            //just wokeup
            //io begins work next clock tick
            sysIO->idle += 1;
            io_is_idle = 0;
            wakeup_io = 0;
            return;
        }
        struct process* currentProcess; 
        queue_dequeue(io_queue, (void**)&currentProcess);
	    movetoiodev(currentProcess->doingIO, currentProcess);
        sysIO->number++;
    }
	
    iodev->current_io_timeLeft--;
    sysIO->busy += 1;
    if (iodev->current_io_timeLeft <= 0) { // if process has completed its I/O, move to ready queue
        io_is_idle = 1; //io goes back to idle
        queue_enqueue(ready_queue, iodev);
        iodev = NULL; // move current process off I/O device
    }
    return;
}

int main( int argc, char *argv[] ) {
    int MAX_LENGTH = 80;
    int line_num, proc_run_time, total_args_read;
    char* proc_name;
    char line[MAX_LENGTH];
    float proc_block_prob;
    
    // If three arguments not included, complain and exit
    if ( argc != 3 ) {
        return (1);
    }
	
    // If 2nd argument isn't -r or -f 
    if (strcmp(argv[1], "-r") == 0) {
        round_robin = 1;
    }
    else if (strcmp(argv[1], "-f") == 0) {
        round_robin = 0;
    }
    else {
        fprintf(stderr, "Usage: ./prsim [-r | -f] file\n");
	    return (1);
    }

    char workingdir[MAX_LENGTH];
    getcwd(workingdir,MAX_LENGTH);

    strcat(workingdir,"/");
    strcat(workingdir,argv[2]);
	
    FILE *file = fopen(workingdir, "r");
    if (!file) {
        return (1);
    }
	
    ready_queue = queue_create();
    io_queue = queue_create();
    // Get each line
    while (fgets(line, MAX_LENGTH, file)) {  
        line_num++;
        proc_name = malloc(MAX_LENGTH);
        total_args_read = sscanf(line, "%s %d %f", proc_name, &proc_run_time, &proc_block_prob);
        if (total_args_read != 3) {
            fprintf(stderr, "Malformed line %s(%d)\n", argv[2], line_num);
            return (1);
	    }
	    if (strlen(proc_name) > 10) {
		    fprintf(stderr, "name is too long %s(%d)\n", argv[2], line_num);
		    return (1);
	    }
	    if (proc_run_time <= 0) {
		    fprintf(stderr, "runtime is not positive integer %s(%d)\n", argv[2], line_num);
		    return (1);
	    }   
	    if (proc_block_prob < 0 || proc_block_prob > 1) {
		    fprintf(stderr, "probability <0 or >1 %s(%d)\n", argv[2], line_num);
		    return (1);
	    }
        //add to ready_queue
        struct process* p = generateProcess(proc_name, proc_run_time,0,0,0,0,proc_block_prob);
        queue_enqueue(ready_queue, p);
    }
	
    //initialize variables
    (void) srandom(12345);
    sysCPU = buildResource("CPU",0,0,0);
    sysIO = buildResource("IO",0,0,0);
    int process_count = ready_queue->count;
    
    setupProcessTable();
    //ready_queue filled
    while (ready_queue->count > 0 || io_queue->count > 0 || cpu != NULL || iodev != NULL) {
        runCPU();
        runio();
        clock++;
        wakeup_io = 0;
    }

    /* print clock time at end */
	printf("\nSystem:\n");
	printf("The wall clock time at which the simulation finished: %d\n", clock-1);

    //calc values off idle + busy time
    calcResourceStats(sysCPU, process_count);
    calcResourceStats(sysIO, process_count);

    /* print cpu + i/o statistics */
    displayResource(sysCPU);
    displayResource(sysIO);
    return 0;
}
