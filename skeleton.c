PROC *cpu;		/* points to process on cpu */
PROC *iodev;		/* points to process on io device */

QUEUE ready = { NULL, NULL };	/* ready queue */
QUEUE io = { NULL, NULL };	/* i/o queue */

PROC *palloc(char *nm, int rt, float pr);
void addtoqueue(QUEUE *q, PROC *p);
void movetocpu(PROC *p);
void movetoiodev(PROC *p);
void runfcfs(void);
void runio(void);
void run();
void rfile(char *fname, FILE *fp);