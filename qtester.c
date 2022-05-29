#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

#define TEST_ASSERT(assert)				\
do {							\
	printf("ASSERT: " #assert " ... ");		\
	if (assert) {					\
		printf("PASS\n");			\
	} else	{					\
		printf("FAIL\n");			\
		exit(1);				\
	}						\
} while(0)

/* Create */
void test_create(void) {
	fprintf(stderr, "*** TEST create ***\n");

	TEST_ASSERT(queue_create() != NULL);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void) {
	int data = 3;
	int data2 = 5;
	int data3 = 456;
	int *ptr;
	queue_t q;

	fprintf(stderr, "*** TEST queue_simple ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_enqueue(q, &data2);
	queue_enqueue(q, &data3);

	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data);
	queue_dequeue(q, (void**)& ptr);
	TEST_ASSERT(ptr == &data2);
	queue_dequeue(q, (void**)& ptr);
	TEST_ASSERT(ptr == &data3);
}

/* Check for errors */
void test_errors(void) {
    int error;
    int *ptr;
    int data = 10;
    int data2 = 90;
    queue_t q;

    fprintf(stderr, "*** TEST errors ***\n");

    q = queue_create();
    // Dequeue from empty queue
    error = queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(error == -1);
    // Enqueue NULL
    error = queue_enqueue(q, NULL);
    TEST_ASSERT(error == -1);
    queue_enqueue(q, &data);
    // Error if node to delete is not found (head)
    error = queue_delete(q, (void*)20);
    TEST_ASSERT(error == -1);
    // Error if node to delete is not found (tail)
    queue_enqueue(q, &data2);
    error = queue_delete(q, (void*)20);
    TEST_ASSERT(error == -1);
}

/* Deletion iterative function */
static int delete_item(queue_t q, void *data, void *arg) {
    int *a = (int*)data;
    int match = (int)(long)arg;
    (void)q; //unused

    if(*a == match)
        queue_delete(q, data);
    return 0;
}

/* Check that deletion works */
void test_queue_delete(void) {
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    size_t i;
    int *ptr;

    fprintf(stderr, "*** TEST queue_delete ***\n");

    /* Initialize the queue and enqueue items */
    q = queue_create();
    for(i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);

    // Delete in the middle
    queue_iterate(q, delete_item, (void*)5, (void**)&ptr);
    TEST_ASSERT(queue_length(q) == 9);
    // Delete the head
    queue_iterate(q, delete_item, (void*)1, (void**)&ptr);
    TEST_ASSERT(queue_length(q) == 8);
    // Delete the tail
    queue_iterate(q, delete_item, (void*)10, (void**)&ptr);
    TEST_ASSERT(queue_length(q) == 7);
    // Trying to delete already-deleted value
    queue_iterate(q, delete_item, (void*)10, (void**)&ptr);
    TEST_ASSERT(queue_length(q) == 7);
}

/* Check that deletion and reenqueue works */
void test_reenqueue(void) {
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    size_t i;
    int *ptr;
    int data2 = 5;

    fprintf(stderr, "*** TEST queue_reenqueue ***\n");

    /* Initialize the queue and enqueue items */
    q = queue_create();
    for(i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);

    // Delete in the middle
    queue_iterate(q, delete_item, (void*)5, (void**)&ptr);
    TEST_ASSERT(queue_length(q) == 9);
    // Enqueue that same item
    queue_enqueue(q, &data2);
    TEST_ASSERT(queue_length(q) == 10);
    // Delete that item again
    queue_iterate(q, delete_item, (void*)5, (void**)&ptr);
    TEST_ASSERT(queue_length(q) == 9);
}

/* Test dequeueing twice, even if there is not another value */
void test_double_dequeue(void) {
    int error;
    int *ptr;
    int data = 10;
    int data2 = 90;
    queue_t q;

    fprintf(stderr, "*** TEST double dequeue ***\n");

    q = queue_create();
    queue_enqueue(q, &data);
    queue_enqueue(q, &data2);
    // Dequeue from empty queue
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(queue_length(q) == 1);
    queue_dequeue(q, (void**)&ptr);
    error = queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(error == -1);
    TEST_ASSERT(queue_length(q) == 0);
}

/* Callback function that increments integer items by a certain value (or delete
 * item if item is value 9) */
static int inc_item(queue_t q, void *data, void *arg) {
    int *a = (int*)data;
    int inc = (int)(long)arg;

    if(*a == 9) 
        queue_delete(q, data);
    else
        *a += inc;
    return 0;
}

/* Callback function that finds a certain item according to its value */
static int find_item(queue_t q, void *data, void *arg) {
    int *a = (int*)data;
    int match = (int)(long)arg;
    (void)q; //unused

    if(*a == match)
        return 1;

    return 0;
}

void test_iterator(void) {
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
    size_t i;
    int *ptr = NULL;

    fprintf(stderr, "*** TEST queue_iterate ***\n");

    /* Initialize the queue and enqueue items */
    q = queue_create();
    for(i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);

    /* Add value '1' to every item of the queue, delete  '9' */
    queue_iterate(q, inc_item, (void*)1, NULL);

    TEST_ASSERT(data[0] == 2);
    assert(queue_length(q) == 9);

    queue_iterate(q, find_item, (void*)5, (void**)&ptr);
    TEST_ASSERT(ptr != NULL);
    TEST_ASSERT(*ptr == 5);
    TEST_ASSERT(ptr == &data[3]);
}

/* Test destroying the queue and then trying to access it */
void test_destroy(void) {
    queue_t q;
    int error;
    int* ptr;

    fprintf(stderr, "*** TEST queue_destroy ***\n");
    /* Initialize the queue */
    q = queue_create();

    // Destroy the empty queue
    error = queue_destroy(q);
    TEST_ASSERT(error == 0);
    // Try to dequeue in destroyed queue
    // error = queue_dequeue(q, (void**)&ptr);
    // TEST_ASSERT(error == -1);
}

/* Test the queue using chars */
void test_character(void) {
    int data = 'a';
    int data2 = 't';
    int* ptr;
    queue_t q;

    fprintf(stderr, "*** TEST character ***\n");

    q = queue_create();
    queue_enqueue(q, &data);
    queue_enqueue(q, &data2);
    TEST_ASSERT(queue_length(q) == 2);
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(queue_length(q) == 1);
}

/* Test creating the queue and immediately iterating */
void test_queue_create_iter(void) {
    queue_t q;
    int error;
    int* ptr;

    fprintf(stderr, "*** TEST queue_create_iter ***\n");

    q = queue_create();
    error = queue_iterate(q, delete_item, (void*)3, (void**)&ptr);
    TEST_ASSERT(error == -1);
}

/* Test the iterate when size of queue is 1 */
void test_one_elem(void) {
    queue_t q;
    char data = 'c';
    char* ptr;

    fprintf(stderr, "*** TEST one_elem ***\n");

    q = queue_create();
    queue_enqueue(q, &data);
    queue_iterate(q, delete_item, (void*)'a', (void**)&ptr);
    TEST_ASSERT(queue_length(q) == 1);
}

/* Test emptying out the queue and then running iterate */
void test_delete_iter(void) {
    queue_t q;
    int data = 5;
    int data2 = 10;
    int* ptr;
    int error;

    fprintf(stderr, "*** TEST delete_iter ***\n");

    q = queue_create();
    queue_enqueue(q, &data);
    queue_enqueue(q, &data2);
    queue_dequeue(q, (void**)ptr);
    queue_dequeue(q, (void**)ptr);
    TEST_ASSERT(queue_length(q) == 0);
    error = queue_iterate(q, delete_item, (void*)6, (void**)&ptr);
    TEST_ASSERT(error == -1);
}

/* Test deleting queue and then remaking it */
void test_delete_create(void) {
    queue_t q;
    int data = 9;
    int data2 = 10;
    int* ptr;

    fprintf(stderr, "*** TEST delete_create ***\n");

    q = queue_create();
    queue_enqueue(q, &data);
    queue_dequeue(q, (void**)&ptr);
    queue_destroy(q);
    q = queue_create();
    queue_enqueue(q, &data2);
    TEST_ASSERT(queue_length(q) == 1);
}

/* Test calling create twice without destroying */
void test_create_create(void) {
    queue_t q;
    int data = 9;
    int data2 = 10;
    int* ptr;

    fprintf(stderr, "*** TEST create_create ***\n");

    q = queue_create();
    queue_enqueue(q, &data);
    q = queue_create();
    queue_enqueue(q, &data2);
    TEST_ASSERT(queue_length(q) == 1);
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(*ptr == 10);
}

/* Test enqueuing an item and requeuing it again at the end */
void test_enqueue_dequeue(void) {
    queue_t q;
    int data = 1;
    int data2 = 5;
    int* ptr;

    fprintf(stderr, "*** TEST enqueue_dequeue ***\n");

    q = queue_create();
    queue_enqueue(q, &data);
    queue_enqueue(q, &data2);
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(*ptr == 1);
    queue_enqueue(q, &data);
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(*ptr == 5);
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(*ptr == 1);
}

/* Test running 2 queues with same data */
void test_multi_queue(void) {
    queue_t q1;
    queue_t q2;
    int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    size_t i;

    fprintf(stderr, "*** TEST multi_queue ***\n");
    
    q1 = queue_create();
    q2 = queue_create();
    
    for(i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q1, &data[i]);
    for(i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q2, &data[i]);

    queue_iterate(q1, inc_item, (void*)1, NULL);
    TEST_ASSERT(data[0] == 2);
    TEST_ASSERT(queue_length(q1) == 9);
    TEST_ASSERT(queue_length(q2) == 10);
    queue_iterate(q2, inc_item, (void*)1, NULL);
    TEST_ASSERT(data[0] == 3);  
}

int main(void) {
    test_create();
    test_queue_simple();
    test_errors();
    test_queue_delete();
    test_reenqueue();
    test_double_dequeue();
    test_iterator();
    test_destroy();
    test_character();
    test_queue_create_iter();
    test_delete_iter();
    test_create_create();
    test_enqueue_dequeue();
    test_multi_queue();

    return 0;
}