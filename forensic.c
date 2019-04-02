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

#include "variables.h"
#include "file.h"
#include "interface.h"

struct forensic fs;

void parsingArg(int argc, char const *argv[], char * envp[]){
    char *token;
    char *delim = ",";

    //char error_message[] = "Usage: forensic [-r] [-h [md5[,sha1[,sha256]]] [-o <outfile>] [-v] <file|dir>\n";

    for(size_t i = 0; i < argc-1; i++){

        //directory
        if(strcmp(argv[i], "-r") == 0){
            i++;
            fs.r_flag = true;
        }

        //output_file
        else if(strcmp(argv[i], "-o") == 0){
            fs.output_file = open(argv[i+1], O_WRONLY|O_CREAT|O_APPEND|O_TRUNC, S_IWGRP);
            printf("Data saved on file %s\n", argv[i+1]);
            i++;
        }

        //execution register
        else if(strcmp(argv[i], "-v") == 0){
            fs.execution_register = malloc(sizeof(char) * strlen(getenv("LOGFILENAME")));
            if(fs.execution_register == NULL){
                perror("execution_register");
                exit(1);
            }

            strcpy (fs.execution_register, getenv("LOGFILENAME="));
            
            printf("Execution records saved on file %s\n", fs.execution_register);
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
    fs.name = (char *) malloc(sizeof(char) * strlen(argv[argc-1]));
    strcpy(fs.name, argv[argc-1]);

    if(stat(fs.name, &fs.last) != 0){
        perror("file|directory stat");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char const *argv[], char * envp[])
{
    init(&fs);

    //Receber, tratar e guardar os argumentos e variáveis de ambiente.
    parsingArg(argc, argv, envp);

    //Extrair a informação solicitada de apenas um ficheiro e imprimi-la na saída padrão de acordo com os argumentos passados.
    recurs(&fs);

    return 0;
}
