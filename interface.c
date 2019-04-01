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
#include "interface.h"

//parses all perminent information to the son struct
void parse_parent_son(struct forensic *son, struct forensic *parent){

    if(parent->output_file != NULL){
        son->output_file = (char *) malloc(sizeof(char)*(strlen(parent->output_file)+strlen(parent->name)+1));
        if(son->output_file == NULL){
            perror("son->output");
            exit(1);
        }
        strcpy(son->output_file, parent->output_file);
    }

    if(parent->execution_register != NULL){
        son->execution_register = (char *) malloc(sizeof(char)*strlen(parent->execution_register));
         if(son->execution_register == NULL){
            perror("son->execution_register");
            exit(1);
        }
        strcpy(son->execution_register, parent->execution_register);
    }

    son->md5 = parent->md5;
    son->sha1 = parent->sha1;
    son->sha256 = parent->sha256;
    son->r_flag = parent->r_flag;

    //it can be a file or a dir.
    if(stat(son->name, &son->last) != 0){
        perror("stat son");
        exit(1);
    }
}

void recurs(struct forensic *parent){
    //if dir
    if((parent->last.st_mode & S_IFMT) == S_IFDIR){
        
        struct dirent *de;
        DIR *dr = opendir(parent->name);

        chdir(parent->name);

        if(dr == NULL){
            perror("dr");
            exit(1);
        }
        

        //opens all files in the current directory
        while((de = readdir(dr)) != NULL){
            struct forensic son;

            //inicializes son's variables.
            init(&son);
            
            //saves name
            son.name = (char *) malloc(sizeof(char) * strlen(de->d_name));
            sprintf(son.name, "%s", de->d_name);
            if(son.name[0] == '.'){
                continue;
            }
            
            printf("** %s\n", son.name);

            //parses all perminent information to the son struct
            parse_parent_son(&son, parent);

            //if the element in consideration is an directory
            if((son.last.st_mode & S_IFMT) == S_IFDIR){
                
                //the user wants recursive
                if(parent->r_flag){
                    pid_t pid;
                    //it creates a seperate process to compute it.
                    if((pid = fork()) == 0){
                        recurs(&son);
                        exit(0);
                    }
                }
                //if the user doen't want recursive then it shows nothing
            }
            //if its a file, it print automatically
            else if(((son.last.st_mode & S_IFMT) == S_IFREG)){
                print_data(&son, parent->name);
            }
        }
        closedir(dr);
    }
    //if file
    else{
        print_data(parent, NULL);
    }
}