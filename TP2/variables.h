#pragma once

#include "sope.h"

#define MAX_REQUESTS 100 //increment this value befor delivery

/**
 * Main structs.
 **/

struct requests
{
        tlv_request_t requestArray[MAX_REQUESTS];
        int first;
        int last;
        int itemCount;

        #ifdef __APPLE__
            dispatch_semaphore_t semafore;
        #else
            sem_t semafore;
        #endif
};

struct server
{
        int tnum;
        char password[MAX_PASSWORD_LEN+1];
};

/**
 * Server data.
 **/

//all the variables necessary to the queue
extern struct requests queue;

//munber of threads to create
//Admin password
extern struct server host;

//pointer for the dynamic pthread_t array  
extern pthread_t * bank_office;

//bank accounts struct array
extern bank_account_t bank_account[MAX_BANK_ACCOUNTS];

//server fifo identifier
extern int srv_fifo_id;

//sever shutdown flag
extern bool server_shutdown;