#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char const *argv[], char * envp[])
{
    
    char *file_name;
    char *directory;
    char *output_file;
    char *execution_register = NULL;
    bool md5 = false; 
    bool sha1 = false;
    bool sha256 = false;
    char *token;
    char *delim = ",";
    bool dir_flag = false;

    //char error_message[] = "Usage: forensic [-r] [-h [md5[,sha1[,sha256]]] [-o <outfile>] [-v] <file|dir>\n";

    printf("%d\n", argc);

    for(size_t i = 0; i < argc; i++){

        //directory
        if(strcmp(argv[i], "-r") == 0){
            directory = (char *) malloc(sizeof(char)*strlen(argv[i+1]));
            if(directory == NULL){
                perror("directory");
                exit(1);
            }

            strcpy(directory, argv[argc-1]);
            i++;
            dir_flag = true;

            continue;
        }

        //output_file
        if(strcmp(argv[i], "-o") == 0){
            output_file = (char *) malloc(sizeof(char) * strlen(argv[i+1]));
            if(output_file == NULL){
                perror("output_file");
                exit(1);
            }

            strcpy(output_file, argv[i+1]);
            i++;

            continue;
        }

        //execution register
        if(strcmp(argv[i], "-v") == 0){
            execution_register = malloc(sizeof(char) * strlen(getenv("LOGFILENAME")));
            if(execution_register == NULL){
                perror("execution_register");
                exit(1);
            }

            strcpy (execution_register, getenv("LOGFILENAME="));

            continue;
        }

        //digital prints
        if (strcmp(argv[i], "-h") == 0) {
            token = strtok((char *)argv[i+1], delim);
            
             while(token != NULL) {

                if(strcmp(token, "md5") == 0) md5 = true;
                if(strcmp(token, "sha1") == 0) sha1 = true;
                if(strcmp(token, "sha256") == 0) sha256 = true;

                token = strtok(NULL, delim);
            }

            printf("\n");

            i++;
            continue;
        }

        //file_name
        if(i == (argc-1) && !dir_flag){
            file_name = (char *) malloc(sizeof(char) * strlen(argv[i]));
            if(file_name == NULL){
                perror("file_name");
                exit(1);
            }

            strcpy(file_name, argv[i]);
        }
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
