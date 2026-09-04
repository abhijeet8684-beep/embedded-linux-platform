#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define EDU_RING_SIZE 8

typedef struct {
	int data[EDU_RING_SIZE];
	int head;
	int tail;
	int count;
} edu_ring;

static void ring_init(edu_ring *ring)
{
	ring->head = 0;
	ring->tail = 0;
	ring->count = 0;
}

static int ring_push(edu_ring *ring, int value)
{
	if (ring->count >= EDU_RING_SIZE)
		return -1;
	ring->data[ring->head] = value;
	ring->head = (ring->head + 1) % EDU_RING_SIZE;
	ring->count++;
	return 0;
}

static int ring_pop(edu_ring *ring)
{
	int value;
	if (ring->count <= 0)
		return -1;
	value = ring->data[ring->tail];
	ring->tail = (ring->tail + 1) % EDU_RING_SIZE;
	ring->count--;
	return value;
}

int main(void)
{
	edu_ring ring;
	int i;

	ring_init(&ring);
	for (i = 0; i < EDU_RING_SIZE; ++i) {
		assert(ring_push(&ring, i) == 0);
	}
	assert(ring_push(&ring, 99) == -1);
	assert(ring_pop(&ring) == 0);
	assert(ring_pop(&ring) == 1);
	assert(ring_push(&ring, 42) == 0);
	assert(ring_pop(&ring) == 2);
	assert(ring_pop(&ring) == 3);
	assert(ring_pop(&ring) == 4);
	assert(ring_pop(&ring) == 5);
	assert(ring_pop(&ring) == 6);
	assert(ring_pop(&ring) == 7);
	assert(ring_pop(&ring) == 42);
	puts("edu_ring_test: PASS");
	return 0;
}
