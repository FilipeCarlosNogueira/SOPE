#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "variables.h"

struct forensic fs;

void init(struct forensic *aux){
    aux->output_file = NULL;
    aux->execution_register = NULL;
    aux->md5 = false;
    aux->sha1 = false;
    aux->sha256 = false;
    aux->r_flag = false;
}

int main(int argc, char const *argv[], char * envp[])
{
    init(&fs);
    char *token;
    char *delim = ",";

    //char error_message[] = "Usage: forensic [-r] [-h [md5[,sha1[,sha256]]] [-o <outfile>] [-v] <file|dir>\n";

    for(size_t i = 0; i < argc-1; i++){

        //directory
        if(strcmp(argv[i], "-r") == 0){

            if(stat(argv[argc-1], &fs.last) != 0){
                perror("directory stat");
                exit(EXIT_FAILURE);
            }
            i++;
            fs.r_flag = true;
        }

        //output_file
        else if(strcmp(argv[i], "-o") == 0){
            fs.output_file = (char *) malloc(sizeof(char) * strlen(argv[i+1]));
            if(fs.output_file == NULL){
                perror("output_file");
                exit(EXIT_FAILURE);
            }

            strcpy(fs.output_file, argv[i+1]);
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

    //it can be a file or a dir. Its not recursive.
    if(!fs.r_flag){
        if(stat(argv[argc-1], &fs.last) != 0){
            perror("file|directory stat");
            exit(EXIT_FAILURE);
        }

        //check if dir.
    }
    

    //file_type
    // pid_t pid;
    // if((pid = fork()) == 0){
    //     execlp("file", "file", argv[1],  NULL);
    // }

    //file_size
    //file_access
    //file_created_date
    //file_modification_da te


    return 0;
}
