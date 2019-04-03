#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/times.h>
#include <signal.h>
#include <sys/wait.h>

#include "variables.h"
#include "file.h"
#include "interface.h"

struct forensic fs;
int n_directories;
int n_files;

// void sigint_handler(int sig)
// {
//     int status;
//     printf("-- %d\n", sig);
//     //CTR-C
//     if(sig == 2){
//         wait(&status);
//         exit(1);
//     }
//     // else{
//     //     printf();
//     // }
// }


void parsingArg(int argc, char const *argv[]){
    char *token;
    char h_aux[100];
    char *delim = ",";

    //char error_message[] = "Usage: forensic [-r] [-h [md5[,sha1[,sha256]]] [-o <outfile>] [-v] <file|dir>\n";

    for(int i = 0; i < argc-1; i++){

        //directory
        if(strcmp(argv[i], "-r") == 0){
            fs.r_flag = true;
        }

        //output_file
        else if(strcmp(argv[i], "-o") == 0){
            fs.output_file = open(argv[i+1], O_RDWR|O_CREAT|O_APPEND|O_TRUNC, S_IWGRP);
            printf("Data saved on file %s\n", argv[i+1]);
            i++;
        }

        //execution register
        else if(strcmp(argv[i], "-v") == 0){
            fs.execution_register = open(getenv("LOGFILENAME"), O_RDWR|O_CREAT|O_APPEND|O_TRUNC, S_IWGRP);
            printf("Execution records saved on file %s\n", getenv("LOGFILENAME"));
        }

        //digital prints
        else if (strcmp(argv[i], "-h") == 0) {
            token = strtok((char *)argv[i+1], delim);

                while(token != NULL) {

                if(strcmp(token, "md5") == 0) fs.md5 = true;
                if(strcmp(token, "sha1") == 0) fs.sha1 = true;
                if(strcmp(token, "sha256") == 0) fs.sha256 = true;

                token = strtok(NULL, delim);
            }

            printf("\n");

            i++;
        }
    }

    //it can be a file or a dir.
    strcpy(fs.name, argv[argc-1]);

    if(lstat(fs.name, &fs.last) != 0){
        perror("file|directory stat");
        exit(EXIT_FAILURE);
    }

    //if user specified the execution register
    if(fs.execution_register != -1){
        fs.start = times(&fs.time);

        strcpy(h_aux, "COMAND forensic ");

        for(int i = 1; i < argc-1; i++){
            strcat(h_aux, argv[i]);
            strcat(h_aux, " ");
        }

        strcat(h_aux, "./");
        strcat(h_aux, argv[argc-1]);
        strcat(h_aux, "\n");

        write(fs.execution_register, h_aux, strlen(h_aux)*sizeof(char));
    }
}

int main(int argc, char const *argv[])
{
    // //criar handler para sigint
    // struct sigaction action;

    // action.sa_handler=sigint_handler;
    // sigemptyset(&action.sa_mask);
    // action.sa_flags=SA_RESTART;

    // //CTR-C
    // sigaction(SIGINT,&action,NULL);
    // sigaction(SIGCHLD,&action,NULL);

    init(&fs);

    //Receber, tratar e guardar os argumentos e variáveis de ambiente.
    parsingArg(argc, argv);

    //Extrair a informação solicitada de apenas um ficheiro e imprimi-la na saída padrão de acordo com os argumentos passados.
    recurs(&fs);

    return 0;
}
