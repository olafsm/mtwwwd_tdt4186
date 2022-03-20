#include "sem.h"
#include "stdlib.h"
#include <stdio.h>

typedef struct BNDBUF{
    int input;
    int output;
    unsigned int counter;
    int* buffer_data;
    SEM* filled_in_buffer;
    SEM* empty_in_buffer;
} BNDBUF;


BNDBUF *bb_init(unsigned int size){

    BNDBUF *buffer = malloc(sizeof(BNDBUF));
    buffer->input = 0;
    buffer->output = 0;
    buffer->counter = 0;
    buffer->buffer_data = malloc(sizeof(int)*size);
    buffer->filled_in_buffer = sem_init(0);
    if(buffer->filled_in_buffer == 0){
        free(buffer->buffer_data);
        return NULL;
    }
    buffer->empty_in_buffer = sem_init(size);
    if(buffer->empty_in_buffer == 0){
        free(buffer->buffer_data);
        sem_del(buffer->empty_in_buffer);
        return NULL;
    }

    return buffer;
}

void bb_del(BNDBUF *bb){

    free(bb->buffer_data);
    sem_del(bb->filled_in_buffer);
    sem_del(bb->empty_in_buffer);
    return;

}

int  bb_get(BNDBUF *bb){
    int get_data;
    wait(bb->filled_in_buffer);
    get_data = bb->buffer_data[bb->counter-1];
    bb->counter--;
    signal(bb->empty_in_buffer);
    return get_data;
}

int bb_add(BNDBUF *bb, int fd) {
    wait(bb->empty_in_buffer);
    bb->buffer_data[bb->counter] = fd;
    bb->counter ++;
    signal(bb->filled_in_buffer);
    return fd;
}

