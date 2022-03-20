#include "sem.h"
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
typedef struct SEM {
    int counter;
    pthread_mutex_t mut;
    pthread_cond_t cond;
} SEM;



SEM *sem_init(int initVal) {
    int n;
    SEM *semaphore = malloc(sizeof(SEM));

    semaphore->counter = initVal;

    semaphore->mut = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    semaphore->cond = (pthread_cond_t)PTHREAD_COND_INITIALIZER;

    n = pthread_mutex_init(&semaphore->mut, NULL);
    
    if(n != 0) {
        free(semaphore);
        return NULL;
    }
    n = pthread_cond_init(&semaphore->cond, NULL);

    if(n != 0){
        pthread_mutex_destroy(&semaphore->mut);
        free(semaphore);
        return NULL;
    }

    return semaphore;
}

int sem_del(SEM *sem){
    int n;
    n = pthread_mutex_destroy(&sem->mut);
    free(sem);
    return n;
}



void wait(SEM *sem){
    /* P (wait) operation.
    * 
    * Attempts to decrement the semaphore value by 1. If the semaphore value 
    * is 0, the operation blocks until a V operation increments the value and 
    * the P operation succeeds.
    *
    * Parameters:
    *
    * sem           handle of the semaphore to decrement
    */
    pthread_mutex_lock(&sem->mut);
    while(sem->counter == 0){
        pthread_cond_wait(&sem->cond, &sem->mut);
    }

    sem->counter --;

    pthread_mutex_unlock(&sem->mut);
    return;
}

void signal(SEM *sem){
    /* V (signal) operation.
    *
    * Increments the semaphore value by 1 and notifies P operations that are 
    * blocked on the semaphore of the change.
    *
    * Parameters:
    *
    * sem           handle of the semaphore to increment
    */
    pthread_mutex_lock(&sem->mut);
    sem->counter ++;
    if(sem->counter == 1 ) {
        pthread_cond_signal(&sem->cond);
    }
    pthread_mutex_unlock(&sem->mut);
    return;
}