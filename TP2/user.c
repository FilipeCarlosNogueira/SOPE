#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#include "sope.h"

tlv_request_t client;
tlv_reply_t reply;

char fifoname[16];

/**
 *
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
 * Creating and opening the user FIFO (tmp/secure_XXXXX, XXXXX=PID).
 * Reading and logging the server reply.
 **/
void receiveFIFO(){
        int pid = getpid();
        char mypid[5];  //ex: 12345
        sprintf(mypid, "%d", pid);

        int fd,n;

        strcpy(fifoname,USER_FIFO_PATH_PREFIX);
        strcpy(fifoname,mypid);


        //open
        if((fd = open(fifoname, O_RDONLY)) == -1) {
                perror("Open user FIFO failed!");
                exit(1);
        }

        //read
        do {
                n = read(fd, &reply, sizeof(reply));

        } while (n<=0);

        //log reply
        if(logReply(STDOUT_FILENO, pid, &reply) < 0) {
                perror("user logReply() failed!");
                exit(1);
        }
}


/**
 * Closes the user FIFO.
 **/
void userFIFOclose(){
        if(unlink(fifoname) == -1) {
                perror("unlink user FIFO failed!");
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

        int fd;
        if((fd = open(SERVER_FIFO_PATH, O_WRONLY)) == 1) {
                reply.value.header.ret_code = 1;
        }

        //send request
        if(write(fd, &client, sizeof(client)) == -1) {
                perror("write() request failed!");
                exit(1);
        }

        //log send request
        if(logRequest(STDOUT_FILENO, client.value.header.pid, &client) < 0) {
                perror("user logRequest() failed!");
                exit(1);
        }

        //create,open,read and log user FIFO,
        //receiveFIFO();

        if(logReply(STDOUT_FILENO, getpid(), &reply) < 0) {
                perror("user logRequest() failed!");
                exit(1);
        }

        //close fifo
        //userFIFOclose();

        return 0;
}
