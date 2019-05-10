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
        if(subfolder != NULL) {
                strcpy(result, subfolder);
                strcat(result, "/");
        }

        //inicialize the current file
        current = new;

        char buff[150];

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
        if(access(current->name, R_OK) == 0) { //read access
                strcat(result, "r");
        }
        if(access(current->name, W_OK) == 0) { //write access
                strcat(result, "w");
        }
        strcat(result, ",");

        //file_created_date
        //doesn't work on linux
        //strftime(buff, 50, "%FT%T", localtime(&current->last.st_birthtime));
        //strcat(result, buff);
        //strcat(result, ",");

        //file_modification_date
        strftime(buff, 50, "%FT%T", localtime(&current->last.st_mtime));
        strcat(result, buff);

        //md5
        if(current->md5) {
                strcat(result, ",");
                strcat(result, algorithm("md5"));
        }
        //sha1
        if(current->sha1) {
                strcat(result, ",");
                strcat(result, algorithm("sha1"));
        }
        //sha256
        if(current->sha256) {
                strcat(result, ",");
                strcat(result, algorithm("sha256"));
        }

        strcat(result, "\n");

        //write result
        //printf("strlen = %ld\n",strlen(result1));
        write(current->output_file, result, strlen(result));
        //close(current->output_file);


        //writes on the execution register file if one was provided
        char exec_reg[50];

        if(current->execution_register != -1) {
                //-inst
                sprintf(buff, "%.2f", ((double) (times(&current->time) - current->start)/sysconf(_SC_CLK_TCK)) * 1000.0);
                strcpy(exec_reg, buff);
                //-pid
                sprintf(buff, " - %d", getpid());
                strcat(exec_reg, buff);
                //-act
                sprintf(buff, " - ANALIZED %s\n", current->name);
                strcat(exec_reg, buff);

                //print on execution_register file
                write(current->execution_register, exec_reg, strlen(exec_reg));
        }
}
