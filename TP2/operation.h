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
 * Funtion that generates the hash operation.
 **/
char *hash(char * str){
        FILE *fp;
        char path[1035];
        char command[500];

        //strcpy(command, "sha256sum ");
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

        for(int i = 0; i < SALT_LEN; i++){
                random = (rand() % strlen(aux));
                sprintf(ch, "%c", aux[random]);
                strcat(salt, ch);
        }
}

/**
 * Manages the:
 *      creation of the salt;
 *      computation of the hash;
 * Attributes the calculated salt and hash to the bank_office[current_ID].
**/ 
void generateCredentials(int bank_account_id){
        char pass_salt[MAX_PASSWORD_LEN+SALT_LEN];

        //bank account salt
        saltGenerator(bank_account[bank_account_id].salt);

        //bank account hash
        strcpy(pass_salt, host.password);
        strcat(pass_salt, bank_account[bank_account_id].salt);

        strcpy(bank_account[bank_account_id].hash, hash(pass_salt));
}

/**
 * Identifies the TPID of the current thread.
**/ 
int currentThreadPID(){
        int arg;

        for(int i = 0; i < host.tnum; i++) {
                if(bank_office[i] == pthread_self()) {
                        arg = i;
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
        bank_account[0].account_id = 0;

        //admin account salt & hash
        generateCredentials(0);

        //admin account balance
        bank_account[0].balance = 0;

        //log creation of admin
        if(logAccountCreation(STDOUT_FILENO, 0, &bank_account[0]) < 0){
                perror("Creation of Admin Acount failed!");
                exit(1);
        }
}

/**
 * --- [OPERATION FUNTIONS] ---
**/

/**
 * 
**/
void createAccount(tlv_request_t request){
        tlv_reply_t reply;

        reply.type = request.type;
        reply.value.header.account_id = request.value.header.account_id;

        if(request.value.header.account_id == 0) {
                if(bank_account[request.value.create.account_id].account_id != request.value.create.account_id) {
                        if(bank_account[request.value.header.account_id].account_id == request.value.header.account_id) {
                                if(request.value.header.account_id != request.value.create.account_id) {
                                        if(request.value.create.balance <= MAX_BALANCE) {
                                                bank_account[request.value.create.account_id].account_id = request.value.create.account_id;

                                                generateCredentials(request.value.create.account_id);

                                                bank_account[request.value.create.account_id].balance = request.value.create.balance;

                                                //auxiliar code
                                                printf("\n[CREATE]\n");
                                                //------------
                                                if(logAccountCreation(STDOUT_FILENO, currentThreadPID(), &bank_account[request.value.create.account_id]) < 0) {
                                                        perror("Creation of Account failed!");
                                                        exit(1);
                                                } else {reply.value.header.ret_code = 0;}
                                        } else {reply.value.header.ret_code = 10;}
                                } else {reply.value.header.ret_code = 8;}
                        } else {reply.value.header.ret_code = 7;}
                } else {reply.value.header.ret_code = 6;}
        }else {reply.value.header.ret_code = 5;}


        reply.length = sizeof(reply);

        if(logReply(STDOUT_FILENO, currentThreadPID(), &reply) < 0) {
                perror("user logRequest() falied!");
                exit(1);
        }
}

/**
 * 
**/
void getBalance(tlv_request_t request){
        tlv_reply_t reply;

        reply.type = request.type;
        reply.value.header.account_id = request.value.header.account_id;

        if(request.value.header.account_id != 0) {
                if(bank_account[request.value.header.account_id].account_id == request.value.header.account_id) {
                        reply.value.header.ret_code = 0;
                        reply.value.balance.balance = bank_account[request.value.header.account_id].balance;
                } else {reply.value.header.ret_code = 7;}
        } else {reply.value.header.ret_code = 5;}


        if(logReply(STDOUT_FILENO, currentThreadPID(), &reply) < 0) {
                perror("user logRequest() falied!");
                exit(1);
        }
}

/**
 * 
**/
void opTransfer(tlv_request_t request){
        tlv_reply_t reply;

        reply.type = request.type;
        reply.value.header.account_id = request.value.header.account_id;

        if(request.value.header.account_id != 0) {
                if(bank_account[request.value.header.account_id].account_id == request.value.header.account_id) {
                        if(bank_account[request.value.transfer.account_id].account_id == request.value.transfer.account_id) {
                                if(request.value.header.account_id != request.value.transfer.account_id) {
                                        if(bank_account[request.value.header.account_id].balance >= request.value.transfer.amount) {
                                                if(request.value.transfer.amount <= MAX_BALANCE) {

                                                        bank_account[request.value.header.account_id].balance-=request.value.transfer.amount;
                                                        bank_account[request.value.transfer.account_id].balance+=request.value.transfer.amount;
                                                        reply.value.header.ret_code = 0;
                                                        reply.value.transfer.balance = request.value.transfer.amount;

                                                } else {reply.value.header.ret_code = 10;}
                                        } else {reply.value.header.ret_code = 9;}
                                } else {reply.value.header.ret_code = 8;}
                        } else {reply.value.header.ret_code = 7;}
                } else {reply.value.header.ret_code = 7;}
        } else {reply.value.header.ret_code = 5;}


        if(logReply(STDOUT_FILENO, currentThreadPID(), &reply) < 0) {
                perror("user logRequest() falied!");
                exit(1);
        }
}

/**
 * 
**/
void Shutdown(tlv_request_t request){
        tlv_reply_t reply;
        reply.type = request.type;
        reply.value.header.account_id = request.value.header.account_id;

        if(request.value.header.account_id == 0) {
                reply.value.shutdown.active_offices = host.tnum--;
        }else {reply.value.header.ret_code = 5;}

        /**
         * FIFO permitions changed to "only read".
        **/
        if(fchmod(srv_fifo_id, 0444) == -1){
                perror("fchmod() failed!");
                exit(1);
        }


        if(logReply(STDOUT_FILENO, currentThreadPID(), &reply) < 0) {
                perror("user logRequest() falied!");
                exit(1);
        }
}

/**
 *
 **/
bool operationManagment(tlv_request_t request){

        //auxiliar code
        printf("\n[OPERATION]\n\n");
        //------------

        //auxiliar code
        printf("\n[REPLY]\n");
        //------------

        switch(request.type) {
        case OP_CREATE_ACCOUNT:
                createAccount(request);
                break;

        case OP_BALANCE:
                getBalance(request);
                break;

        case OP_TRANSFER:
                opTransfer(request);
                break;

        case OP_SHUTDOWN:
                Shutdown(request);
                return true;
                break;

        case __OP_MAX_NUMBER:
                break;

        default:
                break;
        }

        return false;
}