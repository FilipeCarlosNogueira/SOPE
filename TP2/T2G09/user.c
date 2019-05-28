#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#include "sope.h"

tlv_request_t client;
tlv_reply_t reply;

char * fifoname;
int srv_fifo_id;
int usr_fifo_id;

int usr_log;

/**
 * Parses all the shell arguments to their respective variables.
 * Verifies if the theres's a valid arguments number.
 **/
bool parsingCredentials(int argc, char const *argv[]){
        if(argc != 6) {
                printf("Invalid Arguments Number\n");
                return false;
        }

        //acount_id
        if(atoi(argv[1]) > MAX_BANK_ACCOUNTS) {
                printf("Invalid ID Number\n");
                return false;
        }
        else
                client.value.header.account_id = atoi(argv[1]);


        //password
        if(strlen(argv[2]) < MIN_PASSWORD_LEN || strlen(argv[2]) > MAX_PASSWORD_LEN) {
                printf("Invalid Password\n");
                return false;
        }
        else{
                strcpy(client.value.header.password, argv[2]);
        }

        //op_delay_ms
        if(atoi(argv[3]) > MAX_OP_DELAY_MS) {
                printf("Invalid Delay Number\n");
                return false;
        }
        else{
                client.value.header.op_delay_ms = atoi(argv[3]);
        }


        //operation
        int operation = atoi(argv[4]);
        int newID;
        char password[MAX_PASSWORD_LEN + 10];
        switch (operation)
        {
        case 0: //Create acount

                if(strlen(argv[5]) == 0) {
                        printf("Missing Arguments\n");
                        return false;
                }

                //Type
                client.type = 0;

                //new ID
                newID = atoi(strtok((char *) argv[5], " "));
                if(newID > MAX_BANK_ACCOUNTS) {
                        printf("Invalid newID Number\n");
                        return false;
                }
                else
                        client.value.create.account_id = newID;

                //Balance
                client.value.create.balance = atoi(strtok(NULL, " "));

                //Password
                strcpy(password, strtok(NULL, " "));
                if(strlen(password) < MIN_PASSWORD_LEN || strlen(password) > MAX_PASSWORD_LEN) {
                        printf("Invalid newPassword\n");
                        return false;
                }
                else
                        strcpy(client.value.create.password, password);

                break;

        case 1: //Balance, doenst receive args

                client.type = 1;
                break;

        case 2: //Transfer

                if(strlen(argv[5]) == 0) {
                        perror("Missing Arguments");
                        return false;
                }

                //Type
                client.type = 2;

                //Acount ID
                client.value.transfer.account_id = atoi(strtok((char *) argv[5], " "));

                //Amout
                client.value.transfer.amount = atoi(strtok(NULL, " "));

                break;

        case 3://Shutdown, doenst receive args

                client.type = 3;
                break;

        default:
                printf("Invalid Operation Number\n");
                return false;
                break;
        }

        //Process ID
        client.value.header.pid = getpid();

        //Length
        client.length = sizeof(client);

        //Reply
        reply.value.header.account_id = client.value.header.account_id;
        reply.type = client.type;

        return true;
}

/**
 * Creating and opening the user FIFO (tmp/secure_XXXXX, XXXXX=PID) in write only.
 **/
void userFIFOcreate(){

        fifoname = malloc(sizeof(char)*USER_FIFO_PATH_LEN);

        unlink(fifoname);
        
        if(fifoname == NULL) {
                perror("fifoname");
                exit(1);
        }

        //generate the user FIFO's name
        sprintf(fifoname, "%s%d", USER_FIFO_PATH_PREFIX, getpid());

        //creating the server FIFO
        if( mkfifo(fifoname, 0666) < 0) {
                perror("server FIFO failed!");
                exit(1);
        }
}

/**
 * Sends request to the server.
 * Logs the request made.
 **/
void sendRequest(){

        //open server fifo
        if((srv_fifo_id = open(SERVER_FIFO_PATH, O_WRONLY)) < 0) {
                reply.value.header.ret_code = RC_SRV_DOWN;
                if(logReply(usr_log, getpid(), &reply) < 0) {
                        perror("user logReply() failed!");
                        exit(1);
                }
                exit(1);
        }

        //send request
        if(write(srv_fifo_id, &client, sizeof(client)) == -1) {
                perror("write() request failed!");
                exit(1);
        }

        //log send request
        if(logRequest(usr_log, client.value.header.pid, &client) < 0) {
                perror("user logRequest() failed!");
                exit(1);
        }
}

/**
 * Reading the server reply.
 **/
void getReply(){
        //openning the usr FIFO in READ_ONLY
        if((usr_fifo_id = open(fifoname, O_RDONLY)) == -1) {
                perror("Open server FIFO failed!");
                exit(1);
        }

        int n;

        //read
        do {
                n = read(usr_fifo_id, &reply, sizeof(tlv_reply_t));

        } while (n<=0);

        //log reply
        if(logReply(usr_log, getpid(), &reply) < 0) {
                perror("user logReply() failed!");
                exit(1);
        }
}

/**
 * Destroing the user FIFO
 **/
void userFIFOclose(){

        close(usr_fifo_id);

        if(unlink(fifoname) == -1) {
                perror("unlink server FIFO failed!");
                exit(1);
        }
}

/**
 * Main funtion.
 **/
int main(int argc, char const *argv[]){

        //parse user introduced info
        if(!parsingCredentials(argc, argv))
                return -1;

        //open ulog.txt
        if((usr_log = open(USER_LOGFILE, O_WRONLY | O_APPEND | O_CREAT ,0600)) == -1) {
                perror("open ulog.txt failed");
                exit(1);
        }

        //create user FIFO
        userFIFOcreate();

        //send request to server
        sendRequest();

        //get server's reply
        getReply();

        //close user FIFO
        userFIFOclose();

        //close ulog.txt
        close(usr_log);

        return 0;
}
