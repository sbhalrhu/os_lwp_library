
#define next sched_one 
#define TERMINATED 4

/*use this scheduler in lwp.c to implement round robin scheduling.*/
extern struct scheduler RR = {NULL,
                              NULL,
                              rr_admit,
                              rr_remove,
                              rr_next,
                              rr_qlen};