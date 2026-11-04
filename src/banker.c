#include <stdio.h>
#include <string.h>
#include "campus.h"

#define N_PROC 3
#define N_RES  3

static void print_matrix(const char *title, int m[N_PROC][N_RES])
{
    int i, j;
    printf("%s:\n", title);
    for (i = 0; i < N_PROC; i++) {
        printf("  P%d:", i);
        for (j = 0; j < N_RES; j++) {
            printf(" %d", m[i][j]);
        }
        printf("\n");
    }
}

static void print_vec(const char *title, int v[N_RES])
{
    int j;
    printf("%s:", title);
    for (j = 0; j < N_RES; j++) {
        printf(" %d", v[j]);
    }
    printf("\n");
}

/*
 * Banker's safety algorithm: repeatedly pick an unfinished process whose
 * Need fits Work, then pretend it finishes and releases its Allocation.
 * If every process can finish, the state is safe and sequence[] holds order.
 */
static int is_safe(int available[N_RES],
                   int allocation[N_PROC][N_RES],
                   int need[N_PROC][N_RES],
                   int sequence[N_PROC])
{
    int work[N_RES];
    int finish[N_PROC] = {0};
    int count = 0;
    int i, j, found;

    memcpy(work, available, sizeof(work));

    printf("\nSafety check steps:\n");
    while (count < N_PROC) {
        found = 0;
        for (i = 0; i < N_PROC; i++) {
            if (finish[i]) {
                continue;
            }
            for (j = 0; j < N_RES; j++) {
                if (need[i][j] > work[j]) {
                    break;
                }
            }
            if (j == N_RES) {
                printf("  Select P%d (need fits work)\n", i);
                for (j = 0; j < N_RES; j++) {
                    work[j] += allocation[i][j];
                }
                finish[i] = 1;
                sequence[count++] = i;
                found = 1;
                print_vec("  Work now", work);
                break;
            }
        }
        if (!found) {
            return 0; /* no candidate => unsafe */
        }
    }
    return 1;
}

static void run_instance(const char *label,
                         int available[N_RES],
                         int maxm[N_PROC][N_RES],
                         int allocation[N_PROC][N_RES])
{
    int need[N_PROC][N_RES];
    int sequence[N_PROC];
    int i, j;

    printf("\n--- Banker's demo: %s ---\n", label);
    print_vec("Available", available);
    print_matrix("Maximum", maxm);
    print_matrix("Allocation", allocation);

    for (i = 0; i < N_PROC; i++) {
        for (j = 0; j < N_RES; j++) {
            need[i][j] = maxm[i][j] - allocation[i][j];
        }
    }
    print_matrix("Need (= Max - Allocation)", need);

    if (is_safe(available, allocation, need, sequence)) {
        printf("SAFE sequence: ");
        for (i = 0; i < N_PROC; i++) {
            printf("P%d%s", sequence[i], i + 1 == N_PROC ? "\n" : " -> ");
        }
    } else {
        printf("UNSAFE state: no safe sequence exists.\n");
    }
}

void run_banker_demo(int which)
{
    /* Small safe instance: each Need fits after prior releases */
    int safe_avail[N_RES] = {3, 2, 2};
    int safe_max[N_PROC][N_RES] = {
        {2, 1, 1},
        {2, 2, 1},
        {2, 1, 2},
    };
    int safe_alloc[N_PROC][N_RES] = {
        {1, 0, 0},
        {1, 1, 0},
        {1, 0, 1},
    };

    /* Same Max/Allocation but Available is zero → unsafe */
    int unsafe_avail[N_RES] = {0, 0, 0};
    int unsafe_max[N_PROC][N_RES] = {
        {2, 1, 1},
        {2, 2, 1},
        {2, 1, 2},
    };
    int unsafe_alloc[N_PROC][N_RES] = {
        {1, 0, 0},
        {1, 1, 0},
        {1, 0, 1},
    };

    if (which == 1) {
        run_instance("SAFE", safe_avail, safe_max, safe_alloc);
    } else if (which == 2) {
        run_instance("UNSAFE", unsafe_avail, unsafe_max, unsafe_alloc);
    } else {
        printf("Error: banker demo which must be 1 (safe) or 2 (unsafe).\n");
    }
}

void run_banker(void)
{
    int choice;

    printf("\n=== Banker's Algorithm (FR5) ===\n");
    printf("  1. Safe-state demo\n");
    printf("  2. Unsafe-state demo\n");
    printf("Choice: ");
    if (scanf("%d", &choice) != 1) {
        printf("Error: expected a number.\n");
        return;
    }

    if (choice == 1 || choice == 2) {
        run_banker_demo(choice);
    } else {
        printf("Error: choose 1 or 2.\n");
    }
}
