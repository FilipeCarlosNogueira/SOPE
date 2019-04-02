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

char *file_type(){
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

    /* close */
    pclose(fp);

    return token;
}

char *algorithm(char *algm){
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
    //in linux the structer is: (hash code) <file_name>
    char *token = strtok(path, " ");

    /* close */
    pclose(fp);

    return token;
}

void print_data(struct forensic *new, char *subfolder){
    char result[250] = "";
    if(subfolder != NULL){
        strcpy(result, subfolder);
        strcat(result, "/");
    }

    //inicialize the current file
    current = new;

    char buff[50];
    time_t now;
 
    //file_name
    strcat(result, current->name);
    strcat(result, ",");

    //file_type
    strcat(result, file_type());
    strcat(result, ",");

    //file_size
    sprintf(buff, "%lld", (long long) current->last.st_size);
    strcat(result, buff);
    strcat(result, ",");

    //file_access
    if(access(current->name, R_OK) == 0){ //read access
        strcat(result, "r"); 
    }
    if(access(current->name, W_OK) == 0){ //write access
        strcat(result, "w"); 
    }
    strcat(result, ",");

    //file_created_date
    now = time(&current->last.st_birthtime);
    strftime(buff, 50, "%FT%T", localtime(&now));
    strcat(result, buff);
    strcat(result, ",");

    //file_modification_date
    now = time(&current->last.st_mtime);
    strftime(buff, 50, "%FT%T", localtime(&now));
    strcat(result, buff);

    //md5
    if(current->md5){
        strcat(result, ",");
        strcat(result, algorithm("md5"));
    }
    //sha1
    if(current->sha1){
        strcat(result, ",");
        strcat(result, algorithm("sha1"));
    }
    //sha256
    if(current->sha256){
        strcat(result, ",");
        strcat(result, algorithm("sha256"));
    }

    strcat(result, "\n");

    //write result
    write(current->output_file, result, sizeof(result));

    //writes on the execution register file if one was provided
    char exec_reg[100];
    
    if(current->execution_register != -1){
        //-inst
        //sprintf(buff, "%d", localtime(&time(NULL)));
        //-pid
        sprintf(buff, "- %d", current->pid);
        strcat(exec_reg, buff);
        //-act
        sprintf(buff, "- ANALIZED %s\n", current->name);
        strcat(exec_reg, buff);

        write(current->execution_register, exec_reg, sizeof(exec_reg));
    }
}