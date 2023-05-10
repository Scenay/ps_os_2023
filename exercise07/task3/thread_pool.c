#include "thread_pool.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <unistd.h>

void* ready(void* pool) {
    thread_pool* p = (thread_pool*) pool;
    while(!p->done) {
        pthread_mutex_lock(p->mutex);
        if(!myqueue_is_empty(p->q)) {
            struct job_queue_entry job = myqueue_pop(p->q);
            pthread_mutex_unlock(p->mutex);
            job.start_routine(job.arg);
            job.id->ready = 1;
            pthread_cond_signal(job.id->var);
        }
        else {
            pthread_mutex_unlock(p->mutex);
        }
    }
    return NULL;
}

void pool_create(thread_pool* pool, size_t size){
    pool->done = 0;
    pool->size = size;
    pthread_mutex_t* mut = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mut, NULL);
    pool->mutex = mut;
    pool->q = malloc(sizeof(*(pool->q)));
    myqueue_init(pool->q);
    pthread_t threads[size];
    for(unsigned long i = 0; i < size; i++) {
        pthread_create(threads + i, NULL, ready, pool);
    }
}



job_id pool_submit(thread_pool* pool, job_function start_routine, job_arg arg) {
    struct job_queue_entry* job = malloc(sizeof(struct job_queue_entry));
    if (job == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        return NULL;
    }
    job->arg = arg;
    job->start_routine = start_routine;
    job_id id = malloc(sizeof(*id));
    if (id == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        return NULL;
    }
    job->id = id;
    pthread_mutex_t* mut = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mut, NULL);
    id->mutex = mut;
    pthread_cond_t* var = malloc(sizeof(pthread_cond_t));
    pthread_cond_init(var, NULL);
    id->var = var;
    id->ready = 0;
    pthread_mutex_lock(pool->mutex);
    myqueue_push(pool->q, *job);
    pthread_mutex_unlock(pool->mutex);
    return id; 
}

void pool_await(job_id id) {
    pthread_mutex_lock(id->mutex);
    while(!id->ready) {
        pthread_cond_wait(id->var, id->mutex);
    }
    pthread_mutex_unlock(id->mutex);
    pthread_mutex_destroy(id->mutex);
    free(id->mutex);
    pthread_cond_destroy(id->var);
    free(id->var);
    free(id);
    return;   
}

void pool_destroy(thread_pool* pool) {
    pthread_mutex_lock(pool->mutex);
    pool->done = 1;
    pthread_mutex_unlock(pool->mutex);
    pthread_mutex_destroy(pool->mutex);
}