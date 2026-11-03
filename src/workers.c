#include <stdio.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "campus.h"

static void report_wait(pid_t pid, int status, const char *label)
{
    printf("%s (pid %d): ", label, (int)pid);
    if (WIFEXITED(status)) {
        printf("normal exit, code=%d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("killed by signal %d\n", WTERMSIG(status));
    } else {
        printf("unexpected status\n");
    }
}

void run_workers(void)
{
    pid_t pid;
    int status;

    printf("\n=== Worker Success & Failure (FR6) ===\n");
    fflush(stdout);

    /* Normal completion */
    pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    if (pid == 0) {
        printf("Child A: finishing normally.\n");
        fflush(stdout);
        _exit(0);
    }
    waitpid(pid, &status, 0);
    report_wait(pid, status, "Worker A");

    /* Controlled simulated failure */
    fflush(stdout);
    pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    if (pid == 0) {
        printf("Child B: simulating crash (null deref).\n");
        fflush(stdout);
        {
            volatile int *crash = NULL;
            *crash = 1;
        }
        _exit(1); /* not reached if signal delivered */
    }
    waitpid(pid, &status, 0);
    report_wait(pid, status, "Worker B");
}

void run_limits(void)
{
    pid_t pid;
    int status;
    struct rlimit lim;

    printf("\n=== Resource Limit Demo (FR7) ===\n");
    fflush(stdout);

    pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        lim.rlim_cur = 1; /* 1 second of CPU */
        lim.rlim_max = 1;
        if (setrlimit(RLIMIT_CPU, &lim) != 0) {
            perror("setrlimit");
            _exit(1);
        }
        printf("Child: RLIMIT_CPU soft/hard = 1 second; entering busy loop...\n");
        fflush(stdout);
        for (;;) {
            /* burn CPU until the kernel sends SIGXCPU / kills the process */
        }
    }

    waitpid(pid, &status, 0);
    report_wait(pid, status, "Limited worker");
    printf("Observed effect: setrlimit(RLIMIT_CPU) capped CPU time; "
           "the busy child was stopped by the kernel (typically SIGXCPU/SIGKILL).\n");
}
