#pragma once 

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#include "sope.h"
#include "variables.h"

/**
 * //-----------//-----------//-----------//-----------//-----------
 * ------------------- [QUEUE VECTOR FUNTIONS] ---------------------
 * //-----------//-----------//-----------//-----------//-----------
**/

/**
 * Inicializes the queue variables for the manipulation of the circular vector (queue).
 **/
void inicializeRequests(){
        queue.first = 0;
        queue.last = -1;
        queue.itemCount = 0;
}

/**
 * Verifies if the queue is empty.
 * Returns true if empty, false otherwise.
 **/
bool isEmpty() {
    return queue.itemCount == 0;
}

/**
 * Verifies if the queue is full.
 * Returns true if full, false otherwise.
 **/
bool isFull() {
    return queue.itemCount == MAX_REQUESTS;
}

/**
 * Inserts a new request in the queue.
 * Implementation of queue insert: 
 *      Its a circular vector. 
 *      When a new request is added at the end of the vector;
 *      If the front index reaches the end of the queue, it verifies if the there is a empty space at the front;
 *      Case there's not a empty space, the funtion won't add the request;
 **/
void insert(tlv_request_t request) {

    if(!isFull()) {

        if(queue.last == MAX_REQUESTS-1) {
                queue.last = -1;
        }

        queue.requestArray[++queue.last] = request;
        queue.itemCount++;
    }
}

/**
 * Removes the the oldest request.
 * Implementation of the queue remove:
 *      First - in, first - out;
 **/
tlv_request_t removeRequest() {
        tlv_request_t request = queue.requestArray[queue.first++];

        if(queue.first == MAX_REQUESTS) {
                queue.first = 0;
        }

        queue.itemCount--;
        return request;
}

/**
 * //-----------//-----------//-----------//-----------//-----------
 * --------------------- [SEMAFORE FUNTIONS] -----------------------
 * //-----------//-----------//-----------//-----------//-----------
**/

/**
 * Inicializes the semafore.
 * Implemented to APPLE and LINUX.
**/
void semafore_init(){
    #ifdef __APPLE__
        queue.semafore = dispatch_semaphore_create(1);
    #else
        if(sem_init(&queue.semafore, 0, 1) == -1){
                perror("Semafore failed!");
                exit(1);
        }
    #endif
}

/**
 * Locks the semafore.
 * Implemented to APPLE and LINUX.
**/
void semafore_wait(){
    #ifdef __APPLE__
        dispatch_semaphore_wait(queue.semafore, DISPATCH_TIME_FOREVER);
    #else
        int r;

        do {
                r = sem_wait(&queue.semafore);
        } while (r == -1 && errno == EINTR);
    #endif
}

/**
 * Unlocks the semafore.
 * Implemented to APPLE and LINUX.
**/
void semafore_post(){
    #ifdef __APPLE__
        dispatch_semaphore_signal(queue.semafore);
    #else
        sem_post(&queue.semafore);
    #endif
}