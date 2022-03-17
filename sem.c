#include "sem.h"

#include <pthread.h>

typedef struct SEM{
    int counter;
    pthread_mutex_t mut;
    pthread_cond_t cond;
}



SEM *semCreate(int initVal){
    int n;
    SEM *semaphore = malloc(sizeof(SEM));

    (*semaphore).counter = initVal;

    (*semaphore).mut = PTHREAD_MUTEX_INITIALIZER;

    n = pthread_mutex_init(&(*semaphore).mut, NULL);
    if(n != 0){
        free(semaphore);
        return NULL;
    }
    n = pthread_cond_init(&(*semaphore).cond, NULL);

    if(n != 0){
        pthread_mutex_destroy(&(*semaphore).mut, NULL);
        free(semaphore);
        return NULL;
    }

    return semaphore;
}

int sem_del(SEM *sem){
    int n;
    n = pthread_mutex_destroy(&(*sem).mut, NULL)
    free(sem);
    return n;
}

void V(SEM *sem){
    pthread_mutex_lock(&(*sem).mut);
    (*sem).counter ++;
    pthread_mutex_unlock(&(*sem).mut);
    return;
}

void P(SEM *sem){
    pthread_mutex_lock(&(*sem).mut);

    while(&(*sem).counter == 0){
        pthread_cond_wait(&(*sem).counter, &(*sem).mut);
    }

    (*sem).counter --;

    pthread_mutex_unlock(&(*sem).mut);
    return;
}

