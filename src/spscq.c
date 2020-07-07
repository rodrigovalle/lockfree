#include "spscq.h"
#include <stdlib.h>
#include <stdio.h>

// x86 gives us some special memory ordering guarantees that other platforms do
// not necessarily provide. Every read is a read-acquire and every write is
// write-release. The fences will be completely removed when compiled for x86

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

// Since only a single thread will be calling enqueue, we don't have to worry
// about conflicting read-modify-write operations. We do have to make sure
// that when q->head is incremented though that the writes into the array have
// also happened by that time and were not reordered by the hardware.
bool spscq_enqueue(spscq_handle_t q, DataT e) {
    // is the queue full? return an error
    // TODO: to improve perf, might want to use a double locked pattern here?
    //  - keep a thread-specific local variable with the last known head position
    //  - if it doesn't make sense, then read it for real and update the last
    //    known position
    if ((q->head - q->tail) == q->cap) {
        return false;
    }

    // prevent this write from happening before we've checked that the queue
    // isn't full
    __atomic_thread_fence(__ATOMIC_ACQUIRE);
    q->buf[q->head & q->mask] = e;

    // release the enqueued element to be read by the consumer thread
    // __ATOMIC_RELEASE so other threads see this head increment last
    __atomic_thread_fence(__ATOMIC_RELEASE);
    q->head += 1;
    return true;
}

bool spscq_dequeue(spscq_handle_t q, DataT* e) {
    // is the queue empty? return an error
    if ((q->head - q->tail) == 0) {
        return false;
    }

    // prevent this write from happening before we've made sure there's an
    // element to dequeue
    __atomic_thread_fence(__ATOMIC_ACQUIRE);
    *e = q->buf[q->tail & q->mask];

    // tail is read by the producer to see if the queue is full
    // increment tail to dequeue the array slot once we are done reading from it
    // __ATOMIC_RELEASE so other threads see this tail increment last
    __atomic_thread_fence(__ATOMIC_RELEASE);
    q->tail += 1;

    return true;
}
