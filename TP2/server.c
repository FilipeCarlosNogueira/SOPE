#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#include "sope.h"

struct server
{
        int tnum;
        char password[MAX_PASSWORD_LEN+1];   
};

struct server host;

pthread_t * bank_office;
bank_account_t bank_acount[MAX_BANK_ACCOUNTS];

//parses the data provided by the arguments of the shell
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
        if(host.tnum > MAX_BANK_OFFICES || host.tnum <= 0)
                printf("Invalid Thread Number\n");
        else{
                bank_office = (pthread_t*) malloc(sizeof(pthread_t)*host.tnum);
                if(bank_office == NULL){
                        perror("bank office array failed!");
                        exit(1);
                }
        }
                printf("%d ",host.tnum);

        //saving the server password
        if(strlen(argv[2]) < MIN_PASSWORD_LEN || strlen(argv[2]) > MAX_PASSWORD_LEN)
                printf("Invalid Password\n");
        else{
                strcpy(host.password, argv[2]);
                printf("%s\n", host.password);
        }

        return true;
}

//parsing request data
// void * parsingRequest(void * arg){
//         return NULL;
// }

/**
* validades the request
* Send to the user the justificafion 
* returns true if is valid and false if is not a valid request
**/
// bool checkRequest(char * request){

// }

//reading user request
int readRequest(int srv_fifo_id){
        int n;
        char request[100];
        
        do {
                n = read (srv_fifo_id, request, 100);
                
                //check if request fills all the requirements
                // if(!checkRequest(request)){
                //         continue;
                // }

        }while (n<=0);

        printf("%s\n", request);
        
        return (n>0); 
}

int main(int argc, char const *argv[]){

        int srv_fifo_id;
        
        //parses the data provided by the arguments of the shell
        if(!parsingArguments(argc, argv))
                return -1;

        //Creation of the Admin Acount
        if(logAccountCreation(STDOUT_FILENO, 0, bank_acount) < 0){
                perror("Creation of Admin Acount failed!");
                exit(1);
        }

        //DELETE BEFORE DELEVERY
        unlink(SERVER_FIFO_PATH);

        //creating the server FIFO
        if( mkfifo(SERVER_FIFO_PATH, 0666) == -1){
                perror("/tmp/secure_srv failed!");
                exit(1);
        }

        //openning the srv FIFO in READ_ONLY
        if((srv_fifo_id = open(SERVER_FIFO_PATH, O_RDONLY)) == -1){
                perror("Open server fifo failed!");
                exit(1);
        }

        
        while(readRequest(srv_fifo_id)){
                //performs the operation especified..
                continue;
        }

        //Destroing the server FIFO
        if(unlink(SERVER_FIFO_PATH) == -1){
                perror("unlink /tmp/secure_srv failed!");
                exit(1);
        }

        
        return 0;
}
