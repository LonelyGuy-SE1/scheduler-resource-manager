#ifndef CAMPUS_H
#define CAMPUS_H

#define MAX_JOBS 20
#define RR_QUANTUM 2

typedef enum {
    JOB_NEW = 0,
    JOB_READY,
    JOB_RUNNING,
    JOB_WAITING,
    JOB_TERMINATED
} JobStatus;

typedef struct {
    int id;
    int arrival;
    int burst;
    int priority;
    JobStatus status;
    int remaining;
    int completion;
    int turnaround;
    int waiting;
} Job;

const char *job_status_name(JobStatus s);

void run_scheduling(void);
void run_scheduling_compare_sample(void); /* non-interactive: sample jobs, all 3 policies */
void run_logger(void);
void run_ipc(void);
void run_banker(void);
void run_banker_demo(int which); /* 1=safe, 2=unsafe; non-interactive */
void run_workers(void);
void run_limits(void);
void run_integrated(void);
void run_cleanup_check(void);

#endif
