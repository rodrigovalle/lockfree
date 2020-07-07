#include "spscq.h"
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>

#define queue_size 1024

typedef struct timespec timespec_t;
typedef struct {
    spscq_t* queue;
    uint64_t limit;
} Args;

void producer(Args* args) {
    for (uint64_t i = 0; i < args->limit; i++) {
        if (!spscq_enqueue(args->queue, 0)) {
            // try again
            i--;
        }
    }
}

void consumer(Args* args) {
    int64_t tmp;
    for (uint64_t i = 0; i < args->limit; i++) {
        if (!spscq_dequeue(args->queue, &tmp)) {
            // try again
            i--;
        }
    }
}

timespec_t time_diff(const timespec_t* start, const timespec_t* end) {
    timespec_t ret;
    if ((end->tv_nsec - start->tv_nsec) < 0) {
        ret.tv_sec = end->tv_sec - start->tv_sec - 1;
        ret.tv_nsec = 1000000000 + end->tv_nsec - start->tv_nsec;
    } else {
        ret.tv_sec = end->tv_sec - start->tv_sec;
        ret.tv_nsec = end->tv_nsec - start->tv_nsec;
    }
    return ret;
}

int main() {
    spscq_t* queue = spscq_init(1024);
    pthread_t tid_producer, tid_consumer;
    pthread_attr_t* attrs = NULL;

    for (int i = 20; i < 30; i ++) {
        timespec_t start, end, diff;
        Args args = {
            .queue = queue,
            .limit = 1 << i,
        };

        printf("Test with %lu elements took...", args.limit);

        //clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
        clock_gettime(CLOCK_REALTIME, &start);
        pthread_create(&tid_producer, attrs, (void* (*)(void *)) &producer, &args);
        pthread_create(&tid_consumer, attrs, (void* (*)(void *)) &consumer, &args);
        pthread_join(tid_producer, NULL);
        pthread_join(tid_consumer, NULL);
        clock_gettime(CLOCK_REALTIME, &end);

        diff = time_diff(&start, &end);
        double duration = diff.tv_sec + diff.tv_nsec / 1000000000.0;
        double throughput = args.limit / duration;
        printf("  \t %.6f s \t %.1f e/s\n", duration, throughput);
    }
}
