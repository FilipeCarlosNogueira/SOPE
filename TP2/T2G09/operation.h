#pragma once

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#ifdef __APPLE__
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif

#include "sope.h"
#include "variables.h"
#include "queue.h"

int srv_log;

/**
 * Funtion that generates the hash operation.
 **/
char *hash(char * str);

/**
 * Generates a random salt.
 **/
void saltGenerator(char* salt);

/**
 * Only works for the bank accounts.
 * Manages the:
 *      creation of the salt;
 *      computation of the hash;
 * Attributes the calculated salt and hash to the bank_office[current_ID].
 **/
void generateBankCredentials(int bank_account_id, char const * password);

/**
 * Identifies the ID of the current thread.
 **/
int currentThreadID();

/**
 * Creation of the Admin Acount.
 * logs its creation.
 **/
void adminAcount();

/**
 * //-----------//-----------//-----------//-----------//-----------
 * -------------------- [OPERATION FUNTIONS] -----------------------
 * //-----------//-----------//-----------//-----------//-----------
 **/

/**
 * Executes the operaton Create.
 * Generates the reply accordingly.
 **/
tlv_reply_t createAccount(tlv_request_t const request);

/**
 * Executes the operaton Balance.
 * Generates the reply accordingly.
 **/
tlv_reply_t getBalance(tlv_request_t const request);

/**
 * Executes the operaton tranfer.
 * Generates the reply accordingly.
 **/
tlv_reply_t opTransfer(tlv_request_t const request);

/**
 * Executes operation Shut Down.
 * Generates the reply.
 * Changes the permition of the server FIFO to "read only".
 **/
tlv_reply_t Shutdown(tlv_request_t const request);

/**
 * Sends the reply to the proper user.
 * Loggs the reply made.
 **/
void sendReply(tlv_reply_t reply, int usr_pid, int thread_pid);

/**
 * Manages the operations based on the request given.
 * Before the execution of the operation, tries to lock the account mutex.
 * This insures that each account runs one operation at a time.
 **/
void operationManagment(tlv_request_t request);
