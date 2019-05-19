#pragma once

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#ifdef __APPLE__
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif

#include "sope.h"
#include "operation.h"
#include "variables.h"
#include "operation.h"

/**
 * //-----------//-----------//-----------//-----------//-----------
 * ------------------- [QUEUE VECTOR FUNTIONS] ---------------------
 * //-----------//-----------//-----------//-----------//-----------
 **/

/**
 * Inicializes the queue variables for the manipulation of the circular vector (queue).
 **/
void inicializeRequests();

/**
 * Verifies if the queue is empty.
 * Returns true if empty, false otherwise.
 **/
bool isEmpty();

/**
 * Verifies if the queue is full.
 * Returns true if full, false otherwise.
 **/
bool isFull();

/**
 * Inserts a new request in the queue.
 * Implementation of queue insert:
 *      Its a circular vector.
 *      When a new request is added at the end of the vector;
 *      If the front index reaches the end of the queue, it verifies if the there is a empty space at the front;
 *      Case there's not a empty space, the funtion won't add the request;
 **/
void insert(tlv_request_t request);

/**
 * Removes the the oldest request.
 * Implementation of the queue remove:
 *      First - in, first - out;
 **/
tlv_request_t removeRequest();
/**
 * //-----------//-----------//-----------//-----------//-----------
 * --------------------- [SEMAFORE FUNTIONS] -----------------------
 * //-----------//-----------//-----------//-----------//-----------
 **/

/**
 * Inicializes the semafore.
 * Implemented to APPLE and LINUX.
 **/
void semafore_init();

/**
 * Tries to unlocks the semafore.
 * Implemented to APPLE and LINUX.
 **/
int semafore_trywait();

/**
 * Locks the semafore.
 * Implemented to APPLE and LINUX.
 **/
void semafore_wait();

/**
 * Unlocks the semafore.
 * Implemented to APPLE and LINUX.
 **/
void semafore_post(int sid);
