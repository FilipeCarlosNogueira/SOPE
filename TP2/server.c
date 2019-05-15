#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#include "sope.h"
#include "operation.h"
#include "queue.h"

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

/**
 * Thread funtion.
 * Extracts the lastest request and validates the operation.
 * Only one thread at a time can access the requests queue.
 **/
void * bankOffice(){

        //auxiliar code
        int arg = currentThreadPID();
        //--------------

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
 * Frirst authentication of the request.
 * Validates the combination of the id of the account and the given password.
 * Returns true is valid, false otherwise.
 **/
bool requestAuthentication(/*tlv_request_t * request*/){

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

        time_t t;
        /* Intializes random number generator */
        srand((unsigned) time(&t));
        
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
