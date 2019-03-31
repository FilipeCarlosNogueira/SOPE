#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h> 
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

void parsingArg(int argc, char const *argv[], char * envp[]){
    char *token;
    char *delim = ",";

    //char error_message[] = "Usage: forensic [-r] [-h [md5[,sha1[,sha256]]] [-o <outfile>] [-v] <file|dir>\n";

    for(size_t i = 0; i < argc-1; i++){

        //directory
        if(strcmp(argv[i], "-r") == 0){
            fs.name = (char *) malloc(sizeof(char) * strlen(argv[argc-1]));
            strcpy(fs.name, argv[argc-1]);

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
        fs.name = (char *) malloc(sizeof(char) * strlen(argv[argc-1]));
        strcpy(fs.name, argv[argc-1]);

        if(stat(argv[argc-1], &fs.last) != 0){
            perror("file|directory stat");
            exit(EXIT_FAILURE);
        }
    }


}

void file_type(){
    FILE *fp;
    char path[1035];
    char command[50];

    strcpy(command, "file ");
    strcat(command, fs.name);

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
    strtok(path, " ");
    printf("%s,", strtok(NULL, "\n"));

    /* close */
    pclose(fp);
}

void algorithm(char *algm){
    FILE *fp;
    char path[1035];
    char command[50];

    strcpy(command, algm);
    strcat(command, "sum ");
    strcat(command, fs.name);

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
    printf("%s,", strtok(path, " "));

    /* close */
    pclose(fp);
}

void print_data(){

    char buff[20];
    time_t now;

    //file_name
    printf("%s,", fs.name);

    //file_type
    file_type();

    //file_size
    printf("%lld,", (long long) fs.last.st_size);

    //file_access

    //file_created_date
    now = time(&fs.last.st_birthtime);
    strftime(buff, 20, "%FT%T", localtime(&now));
    printf("%s,", buff);

    //file_modification_date
    now = time(&fs.last.st_mtime);
    strftime(buff, 20, "%FT%T", localtime(&now));
    printf("%s,", buff);

    //md5
    if(fs.md5){
        algorithm("md5");
    }
    //sha1
    if(fs.sha1){
        algorithm("sha1");
    }
    //sha256
    if(fs.sha256){
        algorithm("sha256");
    }

    printf("\n");
}

int main(int argc, char const *argv[], char * envp[])
{
    init(&fs);

    //Receber, tratar e guardar os argumentos e variáveis de ambiente.
    parsingArg(argc, argv, envp);

    //Extrair a informação solicitada de apenas um ficheiro e imprimi-la na saída padrão de acordo com os argumentos passados.
    print_data();

    //check if file
    //if(fs.last.st_mode & S)


    return 0;
}
