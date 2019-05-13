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

bool parsingCredentials(int argc, char const *argv[]){
        if(argc != 3) {
                printf("Invalid Arguments Number\n");
                return false;
        }

        host.tnum = atoi(argv[1]);
        if(host.tnum > MAX_BANK_OFFICES)
                printf("Invalid Thread Number\n");
        else
                printf("%d ",host.tnum);

        
        if(strlen(argv[2]) < MIN_PASSWORD_LEN || strlen(argv[2]) > MAX_PASSWORD_LEN)
                printf("Invalid Password\n");
        else{
                strcpy(host.password, argv[2]);
                printf("%s\n", host.password);
        }

        return true;
}

//reading user request
int readRequest(int srv_fifo_id, char *request) {
        int n;
        
        do {
                n = read (srv_fifo_id, request, 100);

        }while (n<=0 && *request++ == '\0');
        
        return (n>0); 
}

int main(int argc, char const *argv[]){

        int srv_fifo_id;
        char request[100];
        
        if(!parsingCredentials(argc, argv))
                return -1;

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

        while(readRequest(srv_fifo_id,request)) 
                printf("%s",request);


        //Destroing the server FIFO
        if(unlink(SERVER_FIFO_PATH) == -1){
                perror("unlink /tmp/secure_srv failed!");
                exit(1);
        }

        
        return 0;
}
