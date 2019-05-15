#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

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

struct requests queue;
pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;

struct server host;

pthread_t * bank_office;
bank_account_t bank_account[MAX_BANK_ACCOUNTS];

int srv_fifo_id;

bool server_shutdown = false;

/**
 * ----- Queue funtions ------
 **/
void inicializeRequests(){
        queue.first = 0;
        queue.last = -1;
        queue.itemCount = 0;
}

bool isEmpty() {
        return queue.itemCount == 0;
}

bool isFull() {
        return queue.itemCount == MAX_REQUESTS;
}

void insert(tlv_request_t request) {

        if(!isFull()) {

                if(queue.last == MAX_REQUESTS-1) {
                        queue.last = -1;
                }

                queue.requestArray[++queue.last] = request;
                queue.itemCount++;
        }
}

tlv_request_t removeRequest() {
        tlv_request_t request = queue.requestArray[queue.first++];

        if(queue.first == MAX_REQUESTS) {
                queue.first = 0;
        }

        queue.itemCount--;
        return request;
}
/**
 * ----------------------------
 **/

/**
 * Funtion that generates the hash operation.
 **/
char *hash(char * str){
        FILE *fp;
        char path[1035];
        char command[50];

        strcpy(command, "sha256sum ");
        strcat(command, str);

        /* Open the command for reading. */
        fp = popen(command, "r");
        if (fp == NULL) {
                perror("popen()");
                exit(1);
        }

        /* Read the output a line at a time - output it. */
        if(fgets(path, sizeof(path)-1, fp) == NULL) {
                perror("");
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

// void credentialGenerator(int bank_account_id){
//         char password[MAX_PASSWORD_LEN];

//         if(bank_account_id == 0){
//                 strcpy(password, host.password);
//         }
//         else{
//                 strcpy(password, )
//         }
//         strcat(bank_account[bank_acount_id].salt, "123");
//         strcpy(bank_account[bank_acount_id].hash, hash(bank_account[bank_account_id]));
// }

/**
 * parses the data provided by the arguments of the shell.
 **/
bool parsingArguments(int argc, char const *argv[]){
        if(argc != 3) {
                printf("Invalid Arguments Number\n");
                return false;
        }

        /**
         * creating the bank_office array
         * validating the number of threads to be created
         **/
        host.tnum = atoi(argv[1]);
        if(host.tnum > MAX_BANK_OFFICES || host.tnum <= 0) {
                perror("Invalid Thread Number");
                exit(1);
        }
        else{
                bank_office = (pthread_t*) malloc(sizeof(pthread_t)*host.tnum);
                if(bank_office == NULL) {
                        perror("bank office array failed!");
                        exit(1);
                }
        }
        printf("bank_offices: %d\n",host.tnum); //auxiliar funtion TO BE DELETED

        //saving the server password
        if(strlen(argv[2]) < MIN_PASSWORD_LEN || strlen(argv[2]) > MAX_PASSWORD_LEN) {
                perror("Invalid Password");
                exit(1);
        }
        else{
                strcpy(host.password, argv[2]);
                printf("server password: %s\n", host.password); //auxiliar funtion TO BE DELETED
        }

        return true;
}

/**
 * Creation of the Admin Acount.
 * logs its creation.
 **/
void adminAcount(){
        bank_account[0].account_id = 0;

        strcpy(bank_account[0].hash, host.password);
        strcpy(bank_account[0].salt, "123");

        bank_account[0].balance = 0;

        if(logAccountCreation(STDOUT_FILENO, 0, &bank_account[0]) < 0) {
                perror("Creation of Admin Account failed!");
                exit(1);
        }
}

/**
 * Creating and opening in read only the server FIFO.
 **/
void serverFIFOopen(){
        //DELETE BEFORE DELEVERY
        unlink(SERVER_FIFO_PATH);

        //creating the server FIFO
        if( mkfifo(SERVER_FIFO_PATH, 0666) == -1) {
                perror("server FIFO failed!");
                exit(1);
        }

        //openning the srv FIFO in READ_ONLY
        if((srv_fifo_id = open(SERVER_FIFO_PATH, O_RDONLY)) == -1) {
                perror("Open server FIFO failed!");
                exit(1);
        }
}

void createAccount(tlv_request_t request){
        tlv_reply_t reply;

        if(request.value.header.account_id == 0) {
                if(bank_account[request.value.create.account_id].account_id != request.value.create.account_id) {
                        if(bank_account[request.value.header.account_id].account_id == request.value.header.account_id) {
                                if(request.value.header.account_id != request.value.create.account_id) {
                                        if(request.value.create.balance <= MAX_BALANCE) {
                                                bank_account[request.value.create.account_id].account_id = request.value.create.account_id;

                                                strcpy(bank_account[request.value.create.account_id].hash, request.value.create.password);
                                                strcpy(bank_account[request.value.create.account_id].salt, "123");

                                                bank_account[request.value.create.account_id].balance = request.value.create.balance;

                                                if(logAccountCreation(STDOUT_FILENO, request.value.header.pid, &bank_account[request.value.create.account_id]) < 0) {
                                                        perror("Creation of Account failed!");
                                                        exit(1);
                                                } else {reply.value.header.ret_code = 0;}
                                        } else {reply.value.header.ret_code = 10;}
                                } else {reply.value.header.ret_code = 8;}
                        } else {reply.value.header.ret_code = 7;}
                } else {reply.value.header.ret_code = 6;}
        }else {reply.value.header.ret_code = 5;}


        if(logReply(STDOUT_FILENO, request.value.header.pid, &reply) < 0) {
                perror("user logRequest() falied!");
                exit(1);
        }
}

void getBalance(tlv_request_t request){
        tlv_reply_t reply;

        if(bank_account[request.value.header.account_id].account_id == request.value.header.account_id) {
                reply.value.header.ret_code = 0;
                reply.value.balance.balance = bank_account[request.value.header.account_id].balance;
        } else {reply.value.header.ret_code = 7;}


        if(logReply(STDOUT_FILENO, request.value.header.pid, &reply) < 0) {
                perror("user logRequest() falied!");
                exit(1);
        }
}

void opTransfer(tlv_request_t request){
        tlv_reply_t reply;
        if(bank_account[request.value.header.account_id].account_id == request.value.header.account_id) {
                if(bank_account[request.value.transfer.account_id].account_id == request.value.transfer.account_id) {
                        if(request.value.header.account_id != request.value.transfer.account_id) {
                                if(bank_account[request.value.header.account_id].balance < request.value.transfer.amount) {
                                        if(request.value.transfer.amount > MAX_BALANCE) {

                                                bank_account[request.value.header.account_id].balance-=request.value.transfer.amount;
                                                bank_account[request.value.transfer.account_id].balance+=request.value.transfer.amount;
                                                reply.value.header.ret_code = 0;
                                                reply.value.transfer.balance = request.value.transfer.amount;

                                        } else {reply.value.header.ret_code = 10;}
                                } else {reply.value.header.ret_code = 9;}
                        } else {reply.value.header.ret_code = 8;}
                } else {reply.value.header.ret_code = 7;}
        } else {reply.value.header.ret_code = 7;}


        if(logReply(STDOUT_FILENO, request.value.header.pid, &reply) < 0) {
                perror("user logRequest() falied!");
                exit(1);
        }
}

void Shutdown(tlv_request_t request){
        tlv_reply_t reply;

        if(request.value.header.account_id == 0) {
                reply.value.shutdown.active_offices = server.tnum--;
        }else {reply.value.header.ret_code = 5;}


        if(logReply(STDOUT_FILENO, request.value.header.pid, &reply) < 0) {
                perror("user logRequest() falied!");
                exit(1);
        }
}

/**
 *
 **/
bool operationManagment(tlv_request_t request){

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
                reply.type = request.type;
                return true;
                break;

        case __OP_MAX_NUMBER:
                break;

        default:
                break;
        }
        reply.type = request.type;
        return false;
}

/**
 * Thread funtion.
 * Extracts the lastest request and validates the operation.
 * Only one thread at a time can access the requests queue.
 **/
void * bankOffice(){

        //--- auxiliar code ---
        int arg;

        for(int i = 0; i < host.tnum; i++) {
                if(bank_office[i] == pthread_self()) {
                        arg = i;
                }
        }
        //---------------------

        tlv_request_t request;

        /**
         * Loop only stops when:
         * the operation to shutdown the server is requested
         * &
         * all pendant requests were processed.
         **/
        while(!(server_shutdown && isEmpty())) {

                //locks mutex.
                pthread_mutex_lock(&queueMutex);

                //Infinit loop. The trhead is always looking for a request.
                while (1) {

                        //when there's a request in the queue, the request is removed.
                        if (!isEmpty()) {

                                //removing request from the queue.
                                request = removeRequest();

                                //--- auxiliar code ---
                                write(STDOUT_FILENO, "\n[REQUEST]\n", 11);
                                logRequest(STDOUT_FILENO, arg, &request);
                                //---------------------

                                server_shutdown = operationManagment(request);

                                break;
                        }
                }

                //unlock mutex.
                pthread_mutex_unlock(&queueMutex);
        }

        return NULL;
}

/**
 * Creates all the threads specified in the shell arguments.
 * After each creation, the funtion logs the oppening of the respective bank office.
 **/
void openBankOffices(){

        for (int i = 0; i < host.tnum; i++) {
                //creating bank office
                pthread_create(&bank_office[i], NULL, &bankOffice, NULL);

                //log creation
                logBankOfficeOpen(STDOUT_FILENO, i+1, bank_office[i]);
        }
}

/**
 * Authenticates the request.
 * Returns true is valid, false otherwise.
 **/
bool requestAuthentication(/*tlv_request_t * request*/){
        //TO BE COMPLETED..

        return true;
}

/**
 * Reading user requests.
 **/
void readRequests(){
        int n;

        tlv_request_t request;

        //Inicializes the variables form the queue operations
        inicializeRequests();

        while (!server_shutdown) {
                do {
                        n = read (srv_fifo_id, &request, sizeof(tlv_request_t));

                } while (n<=0);

                //Authentication of the request
                if(requestAuthentication(/*&request*/)) {
                        //add request to the request queue
                        insert(request);
                }
        }
}

/**
 * Closes all the threads specified in the shell arguments.
 * After each close, the funtion logs the closure of the respective bank office.
 **/
void closeBankOffices(){
        for (int i = 0; i < host.tnum; i++) {
                //closes bank office
                pthread_join(bank_office[i], NULL);

                //log closure
                logBankOfficeClose(STDOUT_FILENO, i+1, bank_office[i]);
        }
}

/**
 * Destroing the server FIFO
 **/
void serverFIFOclose(){
        if(unlink(SERVER_FIFO_PATH) == -1) {
                perror("unlink server FIFO failed!");
                exit(1);
        }
}

int main(int argc, char const *argv[]){

        //parses the data provided by the arguments of the shell
        if(!parsingArguments(argc, argv))
                return -1;

        adminAcount();

        serverFIFOopen();

        openBankOffices();

        readRequests();

        closeBankOffices();

        serverFIFOclose();

        return 0;
}
