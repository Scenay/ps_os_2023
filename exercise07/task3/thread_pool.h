#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <stddef.h> // for size_t
#include <stdatomic.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <pthread.h>

typedef void* (*job_function)(void*);
typedef void* job_arg;
typedef struct {
	pthread_mutex_t* mutex;
	pthread_cond_t* var;
	int ready;
} job_id_struct;
typedef job_id_struct* job_id;

struct job_queue_entry {
	// TODO: Design the contents for a queue, which stores jobs.
	job_arg arg;
	job_function start_routine;
	job_id id;
};

/***
 * This is the stub of a simple job queue.
 */
 struct myqueue_entry {
	struct job_queue_entry job;
	STAILQ_ENTRY(myqueue_entry) entries;
};

STAILQ_HEAD(myqueue_head, myqueue_entry);

typedef struct myqueue_head myqueue;

static void myqueue_init(myqueue* q) {
	STAILQ_INIT(q);
}

static bool myqueue_is_empty(myqueue* q) {
	return STAILQ_EMPTY(q);
}

static void myqueue_push(myqueue* q, struct job_queue_entry job) {
	struct myqueue_entry* entry = malloc(sizeof(struct myqueue_entry));
	entry->job = job;
	STAILQ_INSERT_TAIL(q, entry, entries);
}

static struct job_queue_entry myqueue_pop(myqueue* q) {
	assert(!myqueue_is_empty(q));
	struct myqueue_entry* entry = STAILQ_FIRST(q);
	const struct job_queue_entry job = entry->job;
	STAILQ_REMOVE_HEAD(q, entries);
	free(entry);
	return job;
}



/***
 * This is the stub for the thread pool that uses the queue.
 * Implement at LEAST the Prototype functions below.
 */

typedef struct {
	// TODO: Design the contents of a thread pool
	myqueue* q;
	int done;
	int size;
	pthread_mutex_t* mutex;
	//pthread_t* threads;
} thread_pool;

// Prototypes for REQUIRED functions
void pool_create(thread_pool* pool, size_t size);
job_id pool_submit(thread_pool* pool, job_function start_routine,
                   job_arg arg); // You need to define a datatype for the job_id (chose wisely).
void pool_await(job_id id);
void pool_destroy(thread_pool* pool);

#endif