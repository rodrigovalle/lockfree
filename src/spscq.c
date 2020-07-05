#include "spscq.h"
#include <stdlib.h>
#include <stdio.h>

spscq_handle_t spscq_init(size_t qsize) {
    // does using cache aligned memory actually improve performance over malloc?
    // we also want to avoid the producer and consumer sharing a cache line
    // because cache coherency will slow things down

    if (qsize & (qsize - 1)) {
        perror("qsize not a power of 2");
    }

    size_t alloc_size = sizeof(spscq_t) + qsize * sizeof(DataT);
    spscq_handle_t queue = aligned_alloc(CACHE_LINE_SIZE, alloc_size);
    if (queue == NULL) {
        perror("aligned_alloc");
        return NULL;
    }
    queue->cap = qsize;
    queue->mask = qsize - 1;
    queue->head = 0;
    queue->tail = 0;
    return queue;
}

bool spscq_push(spscq_handle_t q, DataT e) {
    // is the queue full? return an error
    if ((q->head - q->tail) == q->cap) {
        return false;
    }
    q->buf[q->head & q->mask] = e;
    q->head += 1;
    return true;
}

bool spscq_pop(spscq_handle_t q, DataT* e) {
    // is the queue empty? return an error
    if ((q->head - q->tail) == 0) {
        return false;
    }
    *e = q->buf[q->tail & q->mask];
    q->tail += 1;
    return true;
}
