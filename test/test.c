#include "spscq.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define assert(expr)                             \
    do {                                         \
        assert_impl((expr), __func__, __LINE__); \
    } while (0)

static inline void assert_impl(bool expr_result, const char* fn_name, int line) {
    if (!expr_result) {
        printf("%s failed an assertion on line %d\n", fn_name, line);
        exit(1);
    }
}

void print_queue(spscq_handle_t queue) {
    printf("buf: %p\n", queue->buf);
    printf("head == buf[%lu]\n", queue->head);
    printf("tail == buf[%lu]\n", queue->tail);
    printf("cap: %lu\n", queue->cap);
}

void print_queue_struct(spscq_handle_t queue) {
    char* base = (char*) queue;
    printf("offsets\n");
    printf("cap: %td\n", ((char*) &queue->cap) - base);
    printf("mask: %td\n", ((char*) &queue->mask) - base);
    printf("head: %td\n", ((char*) &queue->head) - base);
    printf("tail: %td\n", ((char*) &queue->tail) - base);
    printf("buf: %td\n", ((char*) &queue->buf) - base);
}

void test_push() {
    spscq_handle_t queue = spscq_init(1024);
    for (int64_t i = 1; i <= 100; i++) {
        if (!spscq_enqueue(queue, i)) {
            printf("failed to push element %ld\n", i);
            print_queue(queue);
            break;
        }
    }
}

void test_overflow() {
    spscq_handle_t queue = spscq_init(1024);
    for (int64_t i = 1; i <= 1024; i++) {
        if (!spscq_enqueue(queue, i)) {
            printf("failed to push element %ld\n", i);
            print_queue(queue);
            break;
        }
    }
    int64_t j = 1024;
    assert(!spscq_enqueue(queue, j));
}

void test_pop() {
    spscq_handle_t queue = spscq_init(1024);
    spscq_enqueue(queue, 0);
    spscq_enqueue(queue, 1);

    long s, t, u;
    spscq_dequeue(queue, &s);
    assert(s == 0);

    spscq_dequeue(queue, &t);
    assert(t == 1);

    assert(!spscq_dequeue(queue, &u));
}

void test_wraparound() {
    spscq_handle_t queue = spscq_init(16);

    // move the head and tail pointers past the start of the queue
    int64_t disposal;
    spscq_enqueue(queue, 0);
    spscq_enqueue(queue, 1);
    spscq_dequeue(queue, &disposal);
    spscq_dequeue(queue, &disposal);

    // fill the queue
    for (int64_t i = 0; i < 16; i++) {
        assert(spscq_enqueue(queue, i));
    }
    assert(!spscq_enqueue(queue, 0));

    // empty the queue
    for (int64_t i = 0; i < 16; i++) {
        int64_t val;
        assert(spscq_dequeue(queue, &val));
        assert(val == i);
    }
    assert(!spscq_dequeue(queue, &disposal));
    assert(spscq_enqueue(queue, 0));
}

// what happens when head and tail overflow and wraparound?
void test_head_tail_wrapping() {
    // start with 100 elements in the queue
    spscq_handle_t queue = spscq_init(1024);
    queue->head = SIZE_MAX - 100;
    queue->tail = SIZE_MAX - 200;

    // fill the queue, causing head to wrap
    for (int i = 0; i < (1024 - 100); i++) {
        spscq_enqueue(queue, 0);
    }

    // now the queue is full, we expect this to fail
    assert(!spscq_enqueue(queue, 0));
}

int main() {
    //spscq_handle_t queue;
    //print_queue_struct(&queue);
    test_push();
    test_overflow();
    test_pop();
    test_wraparound();
    test_head_tail_wrapping();
}
