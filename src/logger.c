#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "campus.h"

#define LOG_CAPACITY 8
#define MAX_MSG 64

/* Bounded circular buffer shared by producers and the consumer. */
static char queue[LOG_CAPACITY][MAX_MSG];
static int q_count;
static int q_head;
static int q_tail;
static int producers_done;

/* Mutex guards queue state; condvars signal not-full / not-empty. */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
static pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

static void enqueue(const char *msg)
{
    pthread_mutex_lock(&lock);
    while (q_count == LOG_CAPACITY) {
        /* Wait until the consumer frees a slot. */
        pthread_cond_wait(&not_full, &lock);
    }
    strncpy(queue[q_tail], msg, MAX_MSG - 1);
    queue[q_tail][MAX_MSG - 1] = '\0';
    q_tail = (q_tail + 1) % LOG_CAPACITY;
    q_count++;
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&lock);
}

static int dequeue(char *out)
{
    pthread_mutex_lock(&lock);
    while (q_count == 0 && !producers_done) {
        /* Wait for an item, or for producers to finish. */
        pthread_cond_wait(&not_empty, &lock);
    }
    if (q_count == 0 && producers_done) {
        pthread_mutex_unlock(&lock);
        return 0; /* clean shutdown */
    }
    strncpy(out, queue[q_head], MAX_MSG);
    q_head = (q_head + 1) % LOG_CAPACITY;
    q_count--;
    pthread_cond_signal(&not_full);
    pthread_mutex_unlock(&lock);
    return 1;
}

static void *producer(void *arg)
{
    int id = *(int *)arg;
    char msg[MAX_MSG];
    int i;

    for (i = 1; i <= 5; i++) {
        snprintf(msg, sizeof(msg), "producer-%d event-%d: job activity", id, i);
        enqueue(msg);
    }
    return NULL;
}

static void *consumer(void *arg)
{
    FILE *fp = (FILE *)arg;
    char msg[MAX_MSG];

    while (dequeue(msg)) {
        printf("[LOG] %s\n", msg);
        if (fp) {
            fprintf(fp, "%s\n", msg);
        }
    }
    return NULL;
}

void run_logger(void)
{
    pthread_t prod1, prod2, cons;
    int id1 = 1, id2 = 2;
    FILE *fp;

    printf("\n=== Concurrent Activity Logger (FR3) ===\n");
    printf("Bounded queue capacity=%d; 2 producers, 1 consumer; mutex + condvars.\n",
           LOG_CAPACITY);

    q_count = q_head = q_tail = 0;
    producers_done = 0;

    fp = fopen("campuscompute.log", "w");
    if (!fp) {
        printf("Warning: could not open campuscompute.log; logging to stdout only.\n");
    }

    pthread_create(&cons, NULL, consumer, fp);
    pthread_create(&prod1, NULL, producer, &id1);
    pthread_create(&prod2, NULL, producer, &id2);

    pthread_join(prod1, NULL);
    pthread_join(prod2, NULL);

    /* Signal consumer that no more items will arrive, then join it. */
    pthread_mutex_lock(&lock);
    producers_done = 1;
    pthread_cond_broadcast(&not_empty);
    pthread_mutex_unlock(&lock);

    pthread_join(cons, NULL);

    if (fp) {
        fclose(fp);
        printf("Also wrote events to campuscompute.log\n");
    }
    printf("Logger threads finished cleanly (all joined).\n");
}
