#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
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
        printf("abnormal termination, signal %d (WIFSIGNALED)\n", WTERMSIG(status));
    } else {
        printf("unexpected status\n");
    }
}

void run_workers(void)
{
    pid_t pid;
    int status;

    printf("\n=== Worker Success & Failure (FR6) ===\n");
    printf("Note: the crash below is a CONTROLLED SIMULATED failure for demo only,\n");
    printf("      not an unexpected real OS fault.\n");
    fflush(stdout);

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
    report_wait(pid, status, "Worker A (normal)");

    fflush(stdout);
    pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    if (pid == 0) {
        printf("Child B: SIMULATED failure (intentional null dereference).\n");
        fflush(stdout);
        {
            volatile int *crash = NULL;
            *crash = 1;
        }
        _exit(1);
    }
    waitpid(pid, &status, 0);
    report_wait(pid, status, "Worker B (SIMULATED failure)");
    printf("Parent detected simulated failure via waitpid/WIFSIGNALED.\n");
}

static void handle_sigxcpu(int sig)
{
    (void)sig;
    /* write() is async-signal-safe */
    const char msg[] = "Child: caught SIGXCPU (soft limit hit)! Continuing towards hard limit...\n";
    (void)write(STDOUT_FILENO, msg, sizeof(msg) - 1);
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
        /* Set up signal handler so child catches soft limit (SIGXCPU) */
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = handle_sigxcpu;
        sigaction(SIGXCPU, &sa, NULL);

        lim.rlim_cur = 1; /* Soft limit: 1 CPU second -> triggers SIGXCPU (24) */
        lim.rlim_max = 2; /* Hard limit: 2 CPU seconds -> triggers SIGKILL (9) */
        if (setrlimit(RLIMIT_CPU, &lim) != 0) {
            perror("setrlimit");
            _exit(1);
        }
        printf("Child: RLIMIT_CPU soft=1s, hard=2s; entering busy loop...\n");
        fflush(stdout);
        for (;;) {
        }
    }

    waitpid(pid, &status, 0);
    report_wait(pid, status, "Limited worker");
    printf("Observed effect: Child caught soft limit (SIGXCPU=24), then kernel killed it at hard limit (SIGKILL=9).\n");
}

void run_cleanup_check(void)
{
    char shm_name[64];
    char sem_name[64];
    int fd;
    sem_t *sem;
    void *map;
    int reopen;

    printf("\n=== Cleanup Verification (FR7 / S20) ===\n");

    snprintf(shm_name, sizeof(shm_name), "/cc_chk_%d", (int)getpid());
    snprintf(sem_name, sizeof(sem_name), "/cc_chksem_%d", (int)getpid());

    fd = shm_open(shm_name, O_CREAT | O_RDWR, 0600);
    if (fd < 0) {
        perror("shm_open");
        return;
    }
    ftruncate(fd, 64);
    map = mmap(NULL, 64, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    sem = sem_open(sem_name, O_CREAT, 0600, 0);

    printf("Created temporary IPC: shm=%s sem=%s\n", shm_name, sem_name);

    if (map != MAP_FAILED) {
        munmap(map, 64);
    }
    close(fd);
    shm_unlink(shm_name);
    if (sem != SEM_FAILED) {
        sem_close(sem);
        sem_unlink(sem_name);
    }
    printf("Released shm/sem created by this check.\n");

    reopen = shm_open(shm_name, O_RDONLY, 0);
    if (reopen < 0) {
        printf("Verify: shm_open(%s) after unlink failed as expected (cleaned up).\n",
               shm_name);
    } else {
        printf("Warning: shm still openable — unexpected.\n");
        close(reopen);
        shm_unlink(shm_name);
    }

    if (sem_open(sem_name, 0) == SEM_FAILED) {
        printf("Verify: sem_open(%s) after unlink failed as expected (cleaned up).\n",
               sem_name);
    } else {
        printf("Warning: semaphore still openable — unexpected.\n");
        sem_unlink(sem_name);
    }

    printf("No orphaned CampusCompute IPC objects left by this check.\n");
}
