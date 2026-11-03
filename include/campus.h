#ifndef CAMPUS_H
#define CAMPUS_H

#define MAX_JOBS 20
#define RR_QUANTUM 2

typedef struct {
    int id;
    int arrival;
    int burst;
    int priority;
    int remaining;
    int completion;
    int turnaround;
    int waiting;
} Job;

void run_scheduling(void);
void run_logger(void);
void run_ipc(void);
void run_banker(void);
void run_workers(void);
void run_limits(void);

#endif
