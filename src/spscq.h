#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define CACHE_LINE_SIZE 64

typedef int64_t DataT;
typedef struct {
    uint64_t cap;
    uint64_t mask;

    // place these counters on separate cache lines
    uint64_t head __attribute__((aligned(CACHE_LINE_SIZE)));
    uint64_t tail __attribute__((aligned(CACHE_LINE_SIZE)));

    // buffer
    DataT buf[];
} spscq_t;
typedef spscq_t* spscq_handle_t;

spscq_handle_t spscq_init(uint64_t qsize);
bool spscq_free(spscq_handle_t q);
bool spscq_push(spscq_handle_t q, DataT e);
bool spscq_pop(spscq_handle_t q, DataT* e);
