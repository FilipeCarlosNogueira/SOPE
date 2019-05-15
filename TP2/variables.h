#pragma once

#include "sope.h"

#define MAX_REQUESTS 10 //increment this value befor delivery

/**
 * Main structs.
 **/

struct requests
{
        tlv_request_t requestArray[MAX_REQUESTS];
        int first;
        int last;
        int itemCount;
};

struct server
{
        int tnum;
        char password[MAX_PASSWORD_LEN+1];
};

/**
 * Server data.
 **/

extern struct requests queue;
extern pthread_mutex_t queueMutex;

extern struct server host;

extern pthread_t * bank_office;
extern bank_account_t bank_account[MAX_BANK_ACCOUNTS];

extern int srv_fifo_id;

extern bool server_shutdown;