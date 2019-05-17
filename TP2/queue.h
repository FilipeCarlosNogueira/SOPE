#pragma once

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

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
    int sval;

    #ifdef __APPLE__
        //sem_getvalue() doens't exit on mac os
        sval = 0;

        //log semafore sync
        if(logSyncMechSem(srv_log, 0, SYNC_OP_SEM_INIT, SYNC_ROLE_PRODUCER, 0, sval) < 0){
            perror("sem_post logSyncMechSem() failed!"); 
            exit(1);
        }

        queue.semafore = dispatch_semaphore_create(0);

    #else

        //get vaule
        if(sem_getvalue(&queue.semafore, &sval) < 0){
            perror("sem_getvalue() failed!");
            exit(1);
        }

        //log semafore sync
        if(logSyncMechSem(srv_log, 0, SYNC_OP_SEM_INIT, SYNC_ROLE_PRODUCER, 0, sval) < 0){
            perror("sem_init logSyncMechSem() failed!"); 
            exit(1);
        }

        if(sem_init(&queue.semafore, 0, 0) == -1) {
            perror("Semafore failed!");
            exit(1);
        }
    #endif
}

/**
 * Tries to unlocks the semafore.
 * Implemented to APPLE and LINUX.
 **/
int semafore_trywait(){
    #ifdef __APPLE__
        return dispatch_semaphore_wait(queue.semafore,  DISPATCH_TIME_NOW);
    #else
        return sem_trywait(&queue.semafore);
    #endif
}

/**
 * Locks the semafore.
 * Implemented to APPLE and LINUX.
 **/
void semafore_wait(){
    int sval;

    #ifdef __APPLE__

        //sem_getvalue() doens't exit on mac os
        sval = 0;

        //log semafore sync
        if(logSyncMechSem(srv_log, currentThreadID(), SYNC_OP_SEM_WAIT, SYNC_ROLE_CONSUMER, 0, sval) < 0){
            perror("sem_post logSyncMechSem() failed!"); 
            exit(1);
        }
        
        //dispatch_semaphore_wait(queue.semafore, DISPATCH_TIME_FOREVER);
        while(semafore_trywait() != 0){
            if(server_shutdown && isEmpty()) return;
        }

    #else

        //get vaule
        if(sem_getvalue(&queue.semafore, &sval) < 0){
            perror("sem_getvalue() failed!");
            exit(1);
        }

        //log semafore sync
        if(logSyncMechSem(srv_log, currentThreadPID(), SYNC_OP_SEM_WAIT, SYNC_ROLE_CONSUMER, sid, sval) < 0){
            perror("sem_wait logSyncMechSem() failed!"); 
            exit(1);
        }
        
        //semafore wait
        // if(sem_wait(&queue.semafore) < 0){
        //     perror("sem_wait() failed!"); 
        //     exit(1);
        // }
        while(semafore_trywait() != 0){
            if(server_shutdown && isEmpty()) return;
        }


    #endif
}

/**
 * Unlocks the semafore.
 * Implemented to APPLE and LINUX.
 **/
void semafore_post(int sid){

    int sval;
    #ifdef __APPLE__

        //semarofe post
        dispatch_semaphore_signal(queue.semafore);

        //sem_getvalue() doens't exit on mac os
        sval = 0;

        //log semafore sync
        if(logSyncMechSem(srv_log, 0, SYNC_OP_SEM_POST, SYNC_ROLE_PRODUCER, sid, sval) < 0){
            perror("sem_post logSyncMechSem() failed!"); 
            exit(1);
        }

    #else

        //semarofe post
        if(sem_post(&queue.semafore) < 0){
            perror("sem_post() failed!");
            exit(1);
        }

        int sval; 
        //get vaule
        if(sem_getvalue(&queue.semafore, &sval) < 0){
            perror("sem_getvalue() failed!");
            exit(1);
        }

        //log semafore sync
        if(logSyncMechSem(srv_log, 0, SYNC_OP_SEM_POST, SYNC_ROLE_PRODUCER, sid, sval) < 0){
            perror("sem_post logSyncMechSem() failed!"); 
            exit(1);
        }

    #endif
}