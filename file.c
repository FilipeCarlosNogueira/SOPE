#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>

#include "variables.h"
#include "file.h"

/*
* This file computes all the output information regarding a single file. 
*/

struct forensic *current;

void file_type(char *result){
    FILE *fp;
    char path[1035];
    char command[50];

    strcpy(command, "file ");
    strcat(command, current->name);

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
    char *token = strtok(NULL, "\n");
    result = (char *) realloc(result, sizeof(strlen(result)+strlen(token)+1));
    strcat(result, token);

    /* close */
    pclose(fp);
}

char *algorithm(char *algm, char *result){
    FILE *fp;
    char path[1035];
    char command[50];

    strcpy(command, algm);
    strcat(command, "sum ");
    strcat(command, current->name);

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
    char *token = strtok(path, " ");
    result = (char *) realloc(result, strlen(result)+strlen(token));

    /* close */
    pclose(fp);

    return token;
}

void print_data(struct forensic *new, char *subfolder){
    char *result = NULL;
    if(subfolder != NULL){
        result = (char *) malloc(sizeof(char )*strlen(subfolder)+1);
        strcpy(result, subfolder);
        strcat(result, "/");
    }
    else{
        result = (char *) malloc(sizeof(char));
    }

    //inicialize the current file
    current = new;

    FILE *fp;
    char buff[20];
    time_t now;
 
    //file_name
    result = (char *) realloc(result, strlen(result)+strlen(current->name)+1);
    strcat(result, current->name);
    strcat(result, ",");

    //file_type
    file_type(result);
    result = (char *) realloc(result, strlen(result)+1);
    strcat(result, ",");

    //file_size
    char aux[50];
    sprintf(aux, "%lld", (long long) current->last.st_size);
    result = (char *) realloc(result, strlen(result)+strlen(aux)+1);
    strcat(result, aux);
    strcat(result, ",");

    //file_access
    if(access(current->name, R_OK) == 0){ //read access
        result = (char *) realloc(result, strlen(result)+strlen("r")+1);
        strcat(result, "r"); 
    }
    if(access(current->name, W_OK) == 0){ //write access
        result = (char *) realloc(result, strlen(result)+strlen("w")+1);
        strcat(result, "w"); 
    }
    result = (char *) realloc(result, strlen(result)+1);
    strcat(result, ",");

    //file_created_date
    now = time(&current->last.st_birthtime);
    strftime(buff, 20, "%FT%T", localtime(&now));
    result = (char *) realloc(result, strlen(result)+strlen(buff)+1);
    strcat(result, buff);
    strcat(result, ",");

    //file_modification_date
    now = time(&current->last.st_mtime);
    strftime(buff, 20, "%FT%T", localtime(&now));
    result = (char *) realloc(result, strlen(result)+strlen(buff)+1);
    strcat(result, buff);

    //md5
    if(current->md5){
        result = (char *) realloc(result, strlen(result)+1);
        strcat(result, ",");
        algorithm("md5", result);
    }
    //sha1
    if(current->sha1){
        result = (char *) realloc(result, strlen(result)+1);
        strcat(result, ",");
        algorithm("sha1", result);
    }
    //sha256
    if(current->sha256){
        result = (char *) realloc(result, strlen(result)+1);
        strcat(result, ",");
        algorithm("sha256", result);
    }

    result = (char *) realloc(result, strlen(result)+1);
    strcat(result, "\n");

    //check if output file was specified
    if(current->output_file != NULL){
        fp = fopen(current->output_file, "w");
        fprintf(fp, "%s\n", result);
    }
    else{
        printf("%s\n", result);
    }

    free(result);
}