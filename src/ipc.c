#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "campus.h"

typedef struct {
    int job_id;
    int cpu_seconds;
    char note[32];
} SharedPayload;

void run_ipc(void)
{
    char shm_name[64];
    char sem_name[64];
    int fd = -1;
    SharedPayload *shared = MAP_FAILED;
    sem_t *sem = SEM_FAILED;
    pid_t pid;
    int status;

    printf("\n=== Shared-Memory IPC (FR4) ===\n");

    snprintf(shm_name, sizeof(shm_name), "/cc_shm_%d", (int)getpid());
    snprintf(sem_name, sizeof(sem_name), "/cc_sem_%d", (int)getpid());

    fd = shm_open(shm_name, O_CREAT | O_RDWR, 0600);
    if (fd < 0) {
        perror("shm_open");
        return;
    }
    if (ftruncate(fd, sizeof(SharedPayload)) != 0) {
        perror("ftruncate");
        close(fd);
        shm_unlink(shm_name);
        return;
    }
    shared = mmap(NULL, sizeof(SharedPayload), PROT_READ | PROT_WRITE,
                  MAP_SHARED, fd, 0);
    if (shared == MAP_FAILED) {
        perror("mmap");
        close(fd);
        shm_unlink(shm_name);
        return;
    }
    memset(shared, 0, sizeof(*shared));

    sem = sem_open(sem_name, O_CREAT, 0600, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        munmap(shared, sizeof(SharedPayload));
        close(fd);
        shm_unlink(shm_name);
        return;
    }

    fflush(stdout);
    pid = fork();
    if (pid < 0) {
        perror("fork");
        goto cleanup;
    }

    if (pid == 0) {
        /* child writes lab stats into shared memory */
        shared->job_id = 42;
        shared->cpu_seconds = 7;
        strncpy(shared->note, "simulation done", sizeof(shared->note) - 1);
        printf("Child: wrote job_id=%d cpu=%d note=\"%s\"\n",
               shared->job_id, shared->cpu_seconds, shared->note);
        fflush(stdout);
        sem_post(sem);
        _exit(0);
    }

    printf("Parent: waiting on named semaphore...\n");
    if (sem_wait(sem) != 0) {
        perror("sem_wait");
    } else {
        printf("Parent: read job_id=%d cpu=%d note=\"%s\"\n",
               shared->job_id, shared->cpu_seconds, shared->note);
    }

    waitpid(pid, &status, 0);
    printf("Parent: child exited with status %d\n",
           WIFEXITED(status) ? WEXITSTATUS(status) : -1);

cleanup:
    if (sem != SEM_FAILED) {
        sem_close(sem);
        sem_unlink(sem_name);
    }
    if (shared != MAP_FAILED) {
        munmap(shared, sizeof(SharedPayload));
    }
    if (fd >= 0) {
        close(fd);
        shm_unlink(shm_name);
    }
    printf("IPC resources cleaned up.\n");
}
