
#define next sched_one 
#define TERMINATED 4

/*use this scheduler in lwp.c to implement round robin scheduling.*/

void rr_admit(thread new);
void rr_remove(thread victim);
thread rr_next(void);
int rr_qlen(void);

extern struct scheduler RR;
