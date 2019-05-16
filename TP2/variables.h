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

struct bankAccounts
{
    bank_account_t account;
    pthread_mutex_t account_mutex;
};


/**
 * Server data.
 **/

//all the variables necessary to the queue
extern struct requests queue;

//munber of threads to create
//Admin password
extern struct server host;

//bank accounts struct array
extern struct bankAccounts bank_account[MAX_BANK_ACCOUNTS];

//pointer for the dynamic pthread_t array  
extern pthread_t * bank_office;

//server fifo identifier
extern int srv_fifo_id;

//sever shutdown flag
extern bool server_shutdown;