// Bounded lockless wait-free SPSC queue implementation.

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define CACHE_LINE_SIZE 64

// The type of data that the queue holds.
typedef int64_t DataT;

// The queue datastructure.
typedef struct {
    uint64_t cap;
    uint64_t mask;

    // Place these indices on separate cache lines to avoid false sharing.
    // The writer thread updates `head`.
    // The reader thread updates `tail`.
    uint64_t head __attribute__((aligned(CACHE_LINE_SIZE)));
    uint64_t tail __attribute__((aligned(CACHE_LINE_SIZE)));

    // Buffer is allocated using C99 flexible array members.
    DataT buf[];
} spscq_t;

// Opaque pointer to the queue to encapsulate the API.
typedef spscq_t* spscq_handle_t;

spscq_handle_t spscq_init(uint64_t qsize);
bool spscq_free(spscq_handle_t q);
bool spscq_enqueue(spscq_handle_t q, DataT e);
bool spscq_dequeue(spscq_handle_t q, DataT* e);
