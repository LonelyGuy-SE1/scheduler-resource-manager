#include <stdio.h>
#include <string.h>
#include "campus.h"

const char *job_status_name(JobStatus s)
{
    switch (s) {
    case JOB_NEW:        return "NEW";
    case JOB_READY:      return "READY";
    case JOB_RUNNING:    return "RUNNING";
    case JOB_WAITING:    return "WAITING";
    case JOB_TERMINATED: return "TERMINATED";
    default:             return "?";
    }
}

static void load_sample(Job *jobs, int *n)
{
    /* id, arrival, burst, priority — status starts NEW */
    Job sample[] = {
        {1, 0, 5, 2, JOB_NEW, 0, 0, 0, 0},
        {2, 1, 3, 1, JOB_NEW, 0, 0, 0, 0},
        {3, 2, 8, 3, JOB_NEW, 0, 0, 0, 0},
        {4, 3, 6, 2, JOB_NEW, 0, 0, 0, 0},
    };
    *n = 4;
    memcpy(jobs, sample, sizeof(sample));
}

static void print_job_table(const Job *jobs, int n)
{
    int i;
    printf("  ID  Arrival  Burst  Priority  Status\n");
    for (i = 0; i < n; i++) {
        printf("  %-3d %-8d %-6d %-9d %s\n",
               jobs[i].id, jobs[i].arrival, jobs[i].burst, jobs[i].priority,
               job_status_name(jobs[i].status));
    }
}

static int read_jobs(Job *jobs, int *n)
{
    int i, choice;

    printf("\nJob input:\n");
    printf("  1. Use built-in sample (4 jobs)\n");
    printf("  2. Enter jobs manually\n");
    printf("Choice: ");
    if (scanf("%d", &choice) != 1) {
        printf("Error: expected a number.\n");
        return -1;
    }

    if (choice == 1) {
        load_sample(jobs, n);
        printf("Loaded sample jobs (lifecycle starts at NEW):\n");
        print_job_table(jobs, *n);
        return 0;
    }

    if (choice != 2) {
        printf("Error: choose 1 or 2.\n");
        return -1;
    }

    printf("How many jobs (1-%d)? ", MAX_JOBS);
    if (scanf("%d", n) != 1 || *n < 1 || *n > MAX_JOBS) {
        printf("Error: invalid job count.\n");
        return -1;
    }

    for (i = 0; i < *n; i++) {
        printf("Job %d — enter id arrival burst priority: ", i + 1);
        if (scanf("%d %d %d %d",
                  &jobs[i].id, &jobs[i].arrival,
                  &jobs[i].burst, &jobs[i].priority) != 4) {
            printf("Error: malformed input (need 4 integers).\n");
            return -1;
        }
        if (jobs[i].id <= 0 || jobs[i].arrival < 0 ||
            jobs[i].burst <= 0 || jobs[i].priority < 0) {
            printf("Error: id>0, arrival>=0, burst>0, priority>=0 required.\n");
            return -1;
        }
        jobs[i].status = JOB_NEW;
        jobs[i].remaining = 0;
        jobs[i].completion = 0;
        jobs[i].turnaround = 0;
        jobs[i].waiting = 0;
    }
    printf("Jobs accepted (status=NEW):\n");
    print_job_table(jobs, *n);
    return 0;
}

static void reset_metrics(Job *jobs, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        jobs[i].remaining = jobs[i].burst;
        jobs[i].completion = 0;
        jobs[i].turnaround = 0;
        jobs[i].waiting = 0;
        jobs[i].status = JOB_NEW;
    }
}

static void print_results(Job *jobs, int n, const char *algo, const char *rules)
{
    int i;
    double avg_tat = 0, avg_wt = 0;

    printf("\n--- %s ---\n", algo);
    printf("Rules: %s\n", rules);
    printf("ID  Arrival  Burst  Priority  CT   TAT  WT   Status\n");
    for (i = 0; i < n; i++) {
        printf("%-3d %-8d %-6d %-9d %-4d %-4d %-4d %s\n",
               jobs[i].id, jobs[i].arrival, jobs[i].burst, jobs[i].priority,
               jobs[i].completion, jobs[i].turnaround, jobs[i].waiting,
               job_status_name(jobs[i].status));
        avg_tat += jobs[i].turnaround;
        avg_wt += jobs[i].waiting;
    }
    printf("Average turnaround time: %.2f\n", avg_tat / n);
    printf("Average waiting time:    %.2f\n", avg_wt / n);
}

/* Average waiting time only — used for side-by-side comparison. */
static double avg_waiting(const Job *jobs, int n)
{
    int i;
    double sum = 0;
    for (i = 0; i < n; i++) {
        sum += jobs[i].waiting;
    }
    return sum / n;
}

static double avg_turnaround(const Job *jobs, int n)
{
    int i;
    double sum = 0;
    for (i = 0; i < n; i++) {
        sum += jobs[i].turnaround;
    }
    return sum / n;
}

/* Sort index order by arrival, then id (tie-break). */
static void sort_by_arrival(Job *jobs, int n, int *order)
{
    int i, j, tmp;
    for (i = 0; i < n; i++) {
        order[i] = i;
    }
    for (i = 0; i < n - 1; i++) {
        for (j = i + 1; j < n; j++) {
            int a = order[i], b = order[j];
            if (jobs[b].arrival < jobs[a].arrival ||
                (jobs[b].arrival == jobs[a].arrival && jobs[b].id < jobs[a].id)) {
                tmp = order[i];
                order[i] = order[j];
                order[j] = tmp;
            }
        }
    }
}

static void run_fcfs(Job *jobs, int n)
{
    int order[MAX_JOBS];
    int i, time = 0;

    reset_metrics(jobs, n);
    sort_by_arrival(jobs, n, order);

    printf("Lifecycle: NEW -> READY -> RUNNING -> TERMINATED\n");
    printf("Gantt: ");

    for (i = 0; i < n; i++) {
        int idx = order[i];
        if (time < jobs[idx].arrival) {
            time = jobs[idx].arrival;
        }
        jobs[idx].status = JOB_READY;
        jobs[idx].status = JOB_RUNNING;
        printf("| J%d ", jobs[idx].id);
        time += jobs[idx].burst;
        jobs[idx].completion = time;
        jobs[idx].turnaround = jobs[idx].completion - jobs[idx].arrival;
        jobs[idx].waiting = jobs[idx].turnaround - jobs[idx].burst;
        jobs[idx].status = JOB_TERMINATED;
    }
    printf("| %d\n", time);
    print_results(jobs, n, "FCFS",
                  "non-preemptive; tie-break: earlier arrival, then smaller job id");
}

static void run_priority(Job *jobs, int n)
{
    int done[MAX_JOBS] = {0};
    int completed = 0, time = 0;

    reset_metrics(jobs, n);
    printf("Lifecycle: NEW -> READY -> RUNNING -> TERMINATED\n");
    printf("Gantt: ");

    while (completed < n) {
        int best = -1;
        int i;

        for (i = 0; i < n; i++) {
            if (done[i] || jobs[i].arrival > time) {
                continue;
            }
            if (jobs[i].status == JOB_NEW) {
                jobs[i].status = JOB_READY;
            }
            if (best < 0) {
                best = i;
            } else if (jobs[i].priority > jobs[best].priority) {
                best = i;
            } else if (jobs[i].priority == jobs[best].priority) {
                if (jobs[i].arrival < jobs[best].arrival ||
                    (jobs[i].arrival == jobs[best].arrival &&
                     jobs[i].id < jobs[best].id)) {
                    best = i;
                }
            }
        }

        if (best < 0) {
            /* CPU idle: jump clock to next arrival */
            int next = -1;
            for (i = 0; i < n; i++) {
                if (!done[i] && (next < 0 || jobs[i].arrival < jobs[next].arrival)) {
                    next = i;
                }
            }
            time = jobs[next].arrival;
            continue;
        }

        jobs[best].status = JOB_RUNNING;
        printf("| J%d ", jobs[best].id);
        time += jobs[best].burst;
        jobs[best].completion = time;
        jobs[best].turnaround = jobs[best].completion - jobs[best].arrival;
        jobs[best].waiting = jobs[best].turnaround - jobs[best].burst;
        jobs[best].status = JOB_TERMINATED;
        done[best] = 1;
        completed++;
    }
    printf("| %d\n", time);
    print_results(jobs, n, "Non-preemptive Priority",
                  "higher priority number wins; tie-break: earlier arrival, then smaller id");
}

/*
 * Round Robin: fixed quantum RR_QUANTUM. Ready queue is FIFO.
 * Jobs that arrive during a slice are enqueued after the current slice
 * (before a preempted job is re-queued).
 */
static void run_rr(Job *jobs, int n)
{
    int queue[MAX_JOBS * 4];
    int front = 0, rear = 0;
    int in_queue[MAX_JOBS] = {0};
    int completed = 0, time = 0, i;

    reset_metrics(jobs, n);

    for (i = 0; i < n; i++) {
        if (jobs[i].arrival == 0) {
            jobs[i].status = JOB_READY;
            queue[rear++] = i;
            in_queue[i] = 1;
        }
    }

    printf("Lifecycle: NEW -> READY <-> RUNNING -> TERMINATED (preempt -> READY)\n");
    printf("Gantt (quantum=%d): ", RR_QUANTUM);

    if (rear == 0) {
        int earliest = jobs[0].arrival;
        for (i = 1; i < n; i++) {
            if (jobs[i].arrival < earliest) {
                earliest = jobs[i].arrival;
            }
        }
        time = earliest;
        for (i = 0; i < n; i++) {
            if (jobs[i].arrival == time) {
                jobs[i].status = JOB_READY;
                queue[rear++] = i;
                in_queue[i] = 1;
            }
        }
    }

    while (completed < n) {
        int idx, slice;

        if (front == rear) {
            int next = -1;
            for (i = 0; i < n; i++) {
                if (jobs[i].remaining > 0 &&
                    (next < 0 || jobs[i].arrival < jobs[next].arrival)) {
                    next = i;
                }
            }
            if (next < 0) {
                break;
            }
            time = jobs[next].arrival;
            for (i = 0; i < n; i++) {
                if (!in_queue[i] && jobs[i].remaining > 0 && jobs[i].arrival <= time) {
                    jobs[i].status = JOB_READY;
                    queue[rear++] = i;
                    in_queue[i] = 1;
                }
            }
            continue;
        }

        idx = queue[front++];
        jobs[idx].status = JOB_RUNNING;
        slice = jobs[idx].remaining < RR_QUANTUM ? jobs[idx].remaining : RR_QUANTUM;
        printf("| J%d ", jobs[idx].id);
        time += slice;
        jobs[idx].remaining -= slice;

        /* New arrivals during this quantum join the ready queue now. */
        for (i = 0; i < n; i++) {
            if (!in_queue[i] && jobs[i].remaining > 0 && jobs[i].arrival <= time) {
                jobs[i].status = JOB_READY;
                queue[rear++] = i;
                in_queue[i] = 1;
            }
        }

        if (jobs[idx].remaining == 0) {
            jobs[idx].completion = time;
            jobs[idx].turnaround = jobs[idx].completion - jobs[idx].arrival;
            jobs[idx].waiting = jobs[idx].turnaround - jobs[idx].burst;
            jobs[idx].status = JOB_TERMINATED;
            completed++;
            in_queue[idx] = 0;
        } else {
            /* Quantum expired: back to READY at the end of the queue. */
            jobs[idx].status = JOB_READY;
            queue[rear++] = idx;
        }
    }
    printf("| %d\n", time);
    print_results(jobs, n, "Round Robin",
                  "time quantum = 2; ready queue FIFO; tie-break on simultaneous arrival: smaller id");
}

/* Same job set through all three policies — for scheduling comparison (S09). */
static void run_comparison(Job *base, int n)
{
    Job jobs[MAX_JOBS];
    double tat[3], wt[3];

    printf("\n=== Scheduling comparison (same job set) ===\n");
    print_job_table(base, n);

    memcpy(jobs, base, sizeof(Job) * n);
    printf("\n[1/3] FCFS\n");
    run_fcfs(jobs, n);
    tat[0] = avg_turnaround(jobs, n);
    wt[0] = avg_waiting(jobs, n);

    memcpy(jobs, base, sizeof(Job) * n);
    printf("\n[2/3] Round Robin\n");
    run_rr(jobs, n);
    tat[1] = avg_turnaround(jobs, n);
    wt[1] = avg_waiting(jobs, n);

    memcpy(jobs, base, sizeof(Job) * n);
    printf("\n[3/3] Priority\n");
    run_priority(jobs, n);
    tat[2] = avg_turnaround(jobs, n);
    wt[2] = avg_waiting(jobs, n);

    printf("\n--- Comparison summary (same jobs) ---\n");
    printf("%-12s  Avg TAT   Avg WT\n", "Policy");
    printf("%-12s  %7.2f   %6.2f\n", "FCFS", tat[0], wt[0]);
    printf("%-12s  %7.2f   %6.2f\n", "Round Robin", tat[1], wt[1]);
    printf("%-12s  %7.2f   %6.2f\n", "Priority", tat[2], wt[2]);
}

void run_scheduling_compare_sample(void)
{
    Job jobs[MAX_JOBS];
    int n = 0;

    printf("\n=== Scheduling compare (built-in sample) ===\n");
    load_sample(jobs, &n);
    run_comparison(jobs, n);
}

void run_scheduling(void)
{
    Job jobs[MAX_JOBS];
    int n = 0;
    int algo;

    printf("\n=== Job Scheduling (FR1 / FR2) ===\n");
    if (read_jobs(jobs, &n) != 0) {
        return;
    }

    printf("\nAlgorithm:\n");
    printf("  1. FCFS\n");
    printf("  2. Round Robin (quantum %d)\n", RR_QUANTUM);
    printf("  3. Non-preemptive Priority\n");
    printf("  4. Compare all three on this job set\n");
    printf("Choice: ");
    if (scanf("%d", &algo) != 1) {
        printf("Error: expected a number.\n");
        return;
    }

    switch (algo) {
    case 1:
        run_fcfs(jobs, n);
        break;
    case 2:
        run_rr(jobs, n);
        break;
    case 3:
        run_priority(jobs, n);
        break;
    case 4:
        run_comparison(jobs, n);
        break;
    default:
        printf("Error: choose 1, 2, 3, or 4.\n");
        break;
    }
}
