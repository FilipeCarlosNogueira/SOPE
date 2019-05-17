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
#include "operation.h"
#include "queue.h"

/**
 * Server data.
 **/
struct requests queue;

struct server host;

pthread_t * bank_office;
struct bankAccounts bank_account[MAX_BANK_ACCOUNTS];

int srv_fifo_id;

bool server_shutdown = false;
pthread_mutex_t server_shutdown_mutex = PTHREAD_MUTEX_INITIALIZER;

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
 * Creating and opening the server FIFO in read only.
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
                // tlv_reply_t reply;
                // reply.
                perror("Open server FIFO failed!");
                exit(1);
        }
}

/**
 * Thread funtion.
 * Extracts the lastest request and validates the operation.
 * Only one thread at a time can access the requests queue.
 * Writes the reply of the user fifo.
 **/
void * bankOffice(){

        tlv_request_t request;

        /**
         * Loop only stops when:
         *      the operation to shutdown the server is requested
         *      &
         *      all pendant requests were processed.
         **/
        while(!(server_shutdown && isEmpty())) {

                printf("**thread going\n");

                //locks the semaphore
                semafore_wait();

                printf("**sem lock\n");

                //Infinit loop. The trhead is always looking for a request.
                while (1) {

                        //when there's a request in the queue, the request is removed.
                        if (!isEmpty()) {

                                //removing request from the queue.
                                request = removeRequest();

                                operationManagment(request);

                                //break;
                        }
                }
        }

        printf("thread fim\n");

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
 * Compares if the password given in the request is the same has the bank account.
**/
bool validateCredentials(char * request_pass, bank_account_t * bank_account){

        char pass_salt[MAX_PASSWORD_LEN+SALT_LEN];
        char * hash_compare = malloc(HASH_LEN);

        strcpy(pass_salt, request_pass);
        strcat(pass_salt, bank_account->salt);

        strcpy(hash_compare, hash(pass_salt));

        if(strcmp(hash_compare, bank_account->hash) != 0){
                return false;
        }

        return true;
}

/**
 * First authentication of the request.
 * Validates the combination of the id of the account and the given password.
 * Returns true is valid, false otherwise.
 **/
bool requestAuthentication(tlv_request_t * request){
        
        tlv_reply_t reply;
        int flag = 0;

        //If account was not yet created
        if(bank_account[request->value.header.account_id].account.account_id != request->value.header.account_id){
                flag = 1;
        }
        //hash of bank account was not define
        if(strlen(bank_account[request->value.header.account_id].account.hash) == 0){
                flag = 1;
        }
        //salt of bank account was not define
        if(strlen(bank_account[request->value.header.account_id].account.salt) == 0){
                flag = 1;
        }

        //Validate account login Credentials
        if(!validateCredentials(request->value.header.password, &bank_account[request->value.header.account_id].account)){
                flag = 2;
        }

        //If request was not valid 
        if(flag == 1 || flag == 2){
                reply.type = request->type;
                reply.value.header.account_id = request->value.header.account_id;

                //ID not found
                if(flag == 1) reply.value.header.ret_code = RC_ID_NOT_FOUND;

                //login failed
                if(flag == 2) reply.value.header.ret_code = RC_LOGIN_FAIL;

                reply.value.balance.balance = 0;
                reply.length = sizeof(reply);

                //send reply to user
                sendReply(reply, request->value.header.pid, getpid());

                return false;
        }

        return true;
}

/**
 * Reading user requests.
 **/
void readRequests(){
        int n;

        tlv_request_t request;

        //Inicializes the variables for the queue operations
        inicializeRequests();

        while (!server_shutdown) {
                do {
                        n = read (srv_fifo_id, &request, sizeof(tlv_request_t));

                } while (n<=0);

                //Authentication of the request
                if(requestAuthentication(&request)) {
                        //add request to the request queue
                        insert(request);

                        //unlocks the semaphore
                        semafore_post();
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

        close(srv_fifo_id);

        if(unlink(SERVER_FIFO_PATH) == -1) {
                perror("unlink server FIFO failed!");
                exit(1);
        }
}

/**
 * Main funtion.
 **/
int main(int argc, char const *argv[]){
        
        //parses the data provided by the arguments of the shell
        if(!parsingArguments(argc, argv))
                return -1;

        adminAcount();

        time_t t;
        /* Intializes random number generator */
        srand((unsigned) time(&t));

        //inicializes the semafore
        semafore_init();

        //inicializes all the accounts mutexs
        for(int i = 0; i < MAX_BANK_ACCOUNTS; i++){
                pthread_mutex_init (&bank_account[i].account_mutex, NULL);
        }

        serverFIFOopen();

        openBankOffices();

        readRequests();

        closeBankOffices();

        serverFIFOclose();

        return 0;
}
