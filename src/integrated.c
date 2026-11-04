#include <stdio.h>
#include "campus.h"

/* End-to-end walkthrough with a printed summary (S21). */
void run_integrated(void)
{
    printf("\n=== Integrated Demonstration (S21) ===\n");
    printf("Running demos in sequence (no further prompts)...\n");

    printf("\n-- Step 1/7: Scheduling comparison (sample jobs) --\n");
    run_scheduling_compare_sample();

    printf("\n-- Step 2/7: Concurrent logger --\n");
    run_logger();

    printf("\n-- Step 3/7: Shared-memory IPC --\n");
    run_ipc();

    printf("\n-- Step 4/7: Banker's SAFE state --\n");
    run_banker_demo(1);

    printf("\n-- Step 5/7: Banker's UNSAFE state --\n");
    run_banker_demo(2);

    printf("\n-- Step 6/7: Worker success + SIMULATED failure --\n");
    run_workers();

    printf("\n-- Step 7/7: Resource limit + cleanup check --\n");
    run_limits();
    run_cleanup_check();

    printf("\n========== Integrated summary ==========\n");
    printf("Scheduling: FCFS / RR / Priority compared on one job set.\n");
    printf("Logger:     producer-consumer events written; threads joined.\n");
    printf("IPC:        parent/child exchanged payload via POSIX shm + sem.\n");
    printf("Banker:     safe sequence found; unsafe state identified.\n");
    printf("Workers:    normal exit collected; SIMULATED crash detected.\n");
    printf("Limits:     RLIMIT_CPU applied; child stopped by kernel.\n");
    printf("Cleanup:    program-created shm/sem verified unlinked.\n");
    printf("=======================================\n");
}
