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


int srv_log;

/**
 * Funtion that generates the hash operation.
 **/
char *hash(char * str){
        FILE *fp;
        char path[1035];
        char command[500];

        strcpy(command, "echo -n ");
        strcat(command, str);
        strcat(command, " | shasum -a 256 ");

        /* Open the command for reading. */
        fp = popen(command, "r");
        if (fp == NULL) {
                perror("popen()");
                exit(1);
        }

        /* Read the output a line at a time - output it. */
        if(fscanf(fp, "%s", path) == 0) {
                perror("has funtion failed!");
                exit (1);
        }

        //trim path string
        if(path[strlen(path)-1] == '\n')
                path[strlen(path)-1] = '\0';

        //remove file name from path string
        //in linux the structer is: (hash code) <file_name>
        char *token = strtok(path, " ");


        /* close */
        pclose(fp);

        return token;
}

/**
 * Generates a random salt.
 **/
void saltGenerator(char* salt){

        strcpy(salt, "");

        int random;
        char ch[2];
        char aux[] = "abcdefghijklmnopqrstuvwxyz1234567890";

        for(int i = 0; i < SALT_LEN; i++) {
                random = (rand() % strlen(aux));
                sprintf(ch, "%c", aux[random]);
                strcat(salt, ch);
        }
}

/**
 * Only works for the bank accounts.
 * Manages the:
 *      creation of the salt;
 *      computation of the hash;
 * Attributes the calculated salt and hash to the bank_office[current_ID].
 **/
void generateBankCredentials(int bank_account_id){
        char pass_salt[MAX_PASSWORD_LEN+SALT_LEN];

        //bank account salt
        saltGenerator(bank_account[bank_account_id].account.salt);

        //bank account hash
        strcpy(pass_salt, host.password);
        strcat(pass_salt, bank_account[bank_account_id].account.salt);

        strcpy(bank_account[bank_account_id].account.hash, hash(pass_salt));
}

/**
 * Identifies the TPID of the current thread.
 **/
int currentThreadPID(){
        int arg;

        for(int i = 0; i < host.tnum; i++) {
                if(bank_office[i] == pthread_self()) {
                        arg = i+1;
                }
        }

        return arg;
}

/**
 * Creation of the Admin Acount.
 * logs its creation.
 **/
void adminAcount(){

        //admin account ID
        bank_account[0].account.account_id = 0;

        //admin account salt & hash
        generateBankCredentials(0);

        //admin account balance
        bank_account[0].account.balance = 0;

        //--- auxiliar code ---
        if(logAccountCreation(STDOUT_FILENO, 0, &bank_account[0].account) < 0) {
                perror("Creation of Admin Acount failed!");
                exit(1);
        }
        //---------------------

        //log creation of admin
        if(logAccountCreation(srv_log, 0, &bank_account[0].account) < 0) {
                perror("Creation of Admin Acount failed!");
                exit(1);
        }
}

/**
 * //-----------//-----------//-----------//-----------//-----------
 * -------------------- [OPERATION FUNTIONS] -----------------------
 * //-----------//-----------//-----------//-----------//-----------
 **/

/**
 * Executes the operaton Create.
 * Generates the reply accordingly.
 **/
tlv_reply_t createAccount(tlv_request_t const request){
        tlv_reply_t reply;

        reply.type = request.type;
        reply.value.header.account_id = request.value.header.account_id;

        if(request.value.header.account_id == 0) {
                if(bank_account[request.value.create.account_id].account.account_id != request.value.create.account_id) {
                        if(bank_account[request.value.header.account_id].account.account_id == request.value.header.account_id) {
                                if(request.value.header.account_id != request.value.create.account_id) {
                                        if(request.value.create.balance <= MAX_BALANCE) {
                                                bank_account[request.value.create.account_id].account.account_id = request.value.create.account_id;

                                                generateBankCredentials(request.value.create.account_id);

                                                bank_account[request.value.create.account_id].account.balance = request.value.create.balance;

                                                //--- auxiliar code ---
                                                printf("\n[CREATE]\n");
                                                if(logAccountCreation(STDOUT_FILENO, currentThreadPID(), &bank_account[request.value.create.account_id].account) < 0) {
                                                        perror("Creation of Account failed!");
                                                        exit(1);
                                                }
                                                //---------------------

                                                if(logAccountCreation(srv_log, currentThreadPID(), &bank_account[request.value.create.account_id].account) < 0) {
                                                        perror("Creation of Account failed!");
                                                        exit(1);
                                                }
                                                reply.value.header.ret_code = 0;
                                        } else {reply.value.header.ret_code = 10;}
                                } else {reply.value.header.ret_code = 8;}
                        } else {reply.value.header.ret_code = 7;}
                } else {reply.value.header.ret_code = 6;}
        }else {reply.value.header.ret_code = 5;}


        reply.length = sizeof(reply);

        return reply;
}

/**
 * Executes the operaton Balance.
 * Generates the reply accordingly.
 **/
tlv_reply_t getBalance(tlv_request_t const request){
        tlv_reply_t reply;

        reply.type = request.type;
        reply.value.header.account_id = request.value.header.account_id;

        if(request.value.header.account_id != 0) {
                if(bank_account[request.value.header.account_id].account.account_id == request.value.header.account_id) {
                        reply.value.header.ret_code = 0;
                        reply.value.balance.balance = bank_account[request.value.header.account_id].account.balance;
                } else {reply.value.header.ret_code = 7;}
        } else {reply.value.header.ret_code = 5;}

        reply.length = sizeof(reply);

        return reply;
}

/**
 * Executes the operaton tranfer.
 * Generates the reply accordingly.
 **/
tlv_reply_t opTransfer(tlv_request_t const request){

        //--- auxiliar code ---
        logSyncDelay(STDOUT_FILENO,currentThreadPID(), request.value.header.pid, request.value.header.op_delay_ms);
        //---------------------

        //delay
        if(logSyncDelay(srv_log,currentThreadPID(), request.value.header.pid, request.value.header.op_delay_ms) < 0){
                perror("server logSyncDelay() failed!");
                exit(1);
        }
        //execute delay
        if(usleep(request.value.header.op_delay_ms * 1000) == -1){
                perror("usleep() failed!");
                exit(1);
        }

        tlv_reply_t reply;

        reply.type = request.type;
        reply.value.header.account_id = request.value.header.account_id;

        if(request.value.header.account_id != 0) {
                if(request.value.transfer.account_id > 0) {
                        if(bank_account[request.value.header.account_id].account.account_id == request.value.header.account_id) {
                                if(bank_account[request.value.transfer.account_id].account.account_id == request.value.transfer.account_id) {
                                        if(request.value.header.account_id != request.value.transfer.account_id) {
                                                if(bank_account[request.value.header.account_id].account.balance >= request.value.transfer.amount) {
                                                        if(request.value.transfer.amount <= MAX_BALANCE) {

                                                                bank_account[request.value.header.account_id].account.balance-=request.value.transfer.amount;
                                                                bank_account[request.value.transfer.account_id].account.balance+=request.value.transfer.amount;
                                                                reply.value.header.ret_code = 0;
                                                                reply.value.transfer.balance = request.value.transfer.amount;

                                                        } else {reply.value.header.ret_code = 10;}
                                                } else {reply.value.header.ret_code = 9;}
                                        } else {reply.value.header.ret_code = 8;}
                                } else {reply.value.header.ret_code = 7;}
                        } else {reply.value.header.ret_code = 7;}
                }else {reply.value.header.ret_code = 5;}
        } else {reply.value.header.ret_code = 5;}

        reply.length = sizeof(reply);

        return reply;
}

/**
 * Executes operation Shut Down.
 * Generates the reply.
 * Changes the permition of the server FIFO to "read only"
 **/
tlv_reply_t Shutdown(tlv_request_t const request){

        tlv_reply_t reply;
        reply.type = request.type;
        reply.value.header.account_id = request.value.header.account_id;

        if(request.value.header.account_id == 0) {
                reply.value.shutdown.active_offices = host.tnum--;
        }else {reply.value.header.ret_code = 5;}

        reply.length = sizeof(reply);

        //--- auxiliar code ---
        logDelay(STDOUT_FILENO, request.value.header.pid, request.value.header.op_delay_ms);
        //---------------------

        //shutdown delay
        if(logDelay(srv_log, request.value.header.pid, request.value.header.op_delay_ms) < 0){
                perror("server logDelay() failed!");
                exit(1);
        }
        //execute delay
        if(usleep(request.value.header.op_delay_ms * 1000) == -1){
                perror("usleep() failed!");
                exit(1);
        }

        //FIFO permitions changed to "read only"
        if(fchmod(srv_fifo_id, 0444) == -1) {
                perror("fchmod() failed!");
                exit(1);
        }

        pthread_mutex_lock(&server_shutdown_mutex);
        server_shutdown = true;
        pthread_mutex_unlock(&server_shutdown_mutex);

        return reply;
}

/**
 * Sends the reply to the proper user.
 * Loggs the reply made.
 **/
void sendReply(tlv_reply_t reply, int usr_pid, int thread_pid){
        int usr_fifo_id;
        char *fifoname = malloc(sizeof(char) * USER_FIFO_PATH_LEN);

        //generates the FIFO path
        sprintf(fifoname, "%s%d", USER_FIFO_PATH_PREFIX, usr_pid);

        //opens the user FIFO to WRITE_ONLY
        if((usr_fifo_id = open(fifoname, O_WRONLY)) == -1) {
                reply.value.header.ret_code = 3;
        }

        //send reply
        if(write(usr_fifo_id, &reply, sizeof(reply)) == -1) {
                perror("Write reply to user failed!");
                exit(1);
        }

        //closes user FIFO
        close(usr_fifo_id);

        //--- auxiliar code ---
        printf("\n[REPLY]\n");
        if(logReply(STDOUT_FILENO, thread_pid, &reply) < 0) {
                perror("user logReply() failed!");
                exit(1);
        }
        //---------------------

        //logging reply
        if(logReply(srv_log, thread_pid, &reply) < 0) {
                perror("user logReply() failed!");
                exit(1);
        }
}

/**
 * Manages the operations based on the request given.
 * Before the execution of the operation, tries to lock the account mutex.
 * This insures that each account runs one operation at a time.
 **/
void operationManagment(tlv_request_t request){

        //--- auxiliar code ---
        write(STDOUT_FILENO, "\n[REQUEST]\n", 11);
        logRequest(STDOUT_FILENO, currentThreadPID(), &request);
        //---------------------
        
        //log request
        if(logRequest(srv_log, currentThreadPID(), &request) < 0){
                perror("server logRequest() failed!");
                exit(1);
        }

        tlv_reply_t reply;

        //lock bank account mutex
        pthread_mutex_lock (&bank_account[request.value.header.account_id].account_mutex);

        //delay
        if(request.type != OP_SHUTDOWN){
                //--- auxiliar code ---
                logSyncDelay(STDOUT_FILENO,currentThreadPID(), request.value.header.pid, request.value.header.op_delay_ms);
                //---------------------
                
                //log operation delay
                if(logSyncDelay(srv_log,currentThreadPID(), request.value.header.pid, request.value.header.op_delay_ms) < 0){
                        perror("server logSyncDelay() failed!");
                        exit(1);
                }
                //execute delay
                if(usleep(request.value.header.op_delay_ms * 1000) == -1){
                        perror("usleep() failed!");
                        exit(1);
                }
        }

        switch(request.type) {
                case OP_CREATE_ACCOUNT:
                        reply = createAccount(request);
                        break;

                case OP_BALANCE:
                        reply = getBalance(request);
                        break;

                case OP_TRANSFER:
                        //lock destination bank account mutex
                        pthread_mutex_lock (&bank_account[request.value.transfer.account_id].account_mutex);

                        reply = opTransfer(request);

                        //unclock destination bank account mutex
                        pthread_mutex_unlock (&bank_account[request.value.transfer.account_id].account_mutex);

                        break;

                case OP_SHUTDOWN:

                        reply = Shutdown(request);

                        break;

                case __OP_MAX_NUMBER:
                        reply.type = __OP_MAX_NUMBER;
                        reply.length = sizeof(reply);
                        break;

                default:
                        break;
        }

        //send reply to user
        sendReply(reply, request.value.header.pid, currentThreadPID());

        //unclock bank account mutex
        pthread_mutex_unlock (&bank_account[request.value.header.account_id].account_mutex);
}
