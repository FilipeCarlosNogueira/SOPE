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
        if(host.tnum > MAX_BANK_OFFICES || host.tnum <= 0){
                perror("Invalid Thread Number");
                exit(1);
        }
        else{
                bank_office = (pthread_t*) malloc(sizeof(pthread_t)*host.tnum);
                if(bank_office == NULL){
                        perror("bank office array failed!");
                        exit(1);
                }
        }
        printf("bank_offices: %d\n",host.tnum); //auxiliar funtion TO BE DELETED

        //saving the server password
        if(strlen(argv[2]) < MIN_PASSWORD_LEN || strlen(argv[2]) > MAX_PASSWORD_LEN){
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

        char pass_salt[MAX_PASSWORD_LEN+SALT_LEN];

        //admin account ID
        bank_account[0].account_id = 0;

        //admin account salt
        saltGenerator(bank_account[0].salt);

        //admin account hash
        strcpy(pass_salt, host.password);
        strcat(pass_salt, bank_account[0].salt);

        strcpy(bank_account[0].hash, hash(pass_salt));

        //admin account balance
        bank_account[0].balance = 0;

        //log creation of admin
        if(logAccountCreation(STDOUT_FILENO, 0, &bank_account[0]) < 0){
                perror("Creation of Admin Acount failed!");
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
        if( mkfifo(SERVER_FIFO_PATH, 0666) == -1){
                perror("server FIFO failed!");
                exit(1);
        }

        //openning the srv FIFO in READ_ONLY
        if((srv_fifo_id = open(SERVER_FIFO_PATH, O_RDONLY)) == -1){
                perror("Open server FIFO failed!");
                exit(1);
        }
}

tlv_request_t operationX(tlv_request_t request){
        return request;
}

/**
 * 
**/
bool operationManagment(tlv_request_t request){

        switch(request.type){
                case OP_CREATE_ACCOUNT:
                        operationX(request);
                break;

                case OP_BALANCE:
                        operationX(request);
                break;

                case OP_TRANSFER:
                        operationX(request);
                break;

                case OP_SHUTDOWN:
                        operationX(request);
                break;

                case __OP_MAX_NUMBER:
                        operationX(request);
                break;

                default:
                        operationX(request);
                break;
        }

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

        for(int i = 0; i < host.tnum; i++){
                if(bank_office[i] == pthread_self()){
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
        while(!(server_shutdown && isEmpty())){

                //locks mutex.
                pthread_mutex_lock(&queueMutex);

                //Infinit loop. The trhead is always looking for a request.
                while (1){

                        //when there's a request in the queue, the request is removed.
                        if (!isEmpty()){

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
        
        for (int i = 0; i < host.tnum; i++){
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

        while (!server_shutdown){
                do {
                        n = read (srv_fifo_id, &request, sizeof(tlv_request_t));

                }while (n<=0);

                //Authentication of the request
                if(requestAuthentication(/*&request*/)){
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
        for (int i = 0; i < host.tnum; i++){
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
        if(unlink(SERVER_FIFO_PATH) == -1){
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
