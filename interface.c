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

void parse_parent_son(struct forensic *son, struct forensic *parent){
    //it can be a file or a dir.
    if(stat(son.name, &son.last) != 0){
        perror("file|directory stat");
        exit(EXIT_FAILURE);
    }

    son->output_file = (char *) malloc(sizeof(char)*strlen(parent->output_file));
    strcpy(son->output_file, parent->output_file);

    son->execution_register = (char *) malloc(sizeof(char)*strlen(parent->execution_register));
    strcpy(son->execution_register, parent->execution_register);

    son->md5 = parent->md5;
    son->sha1 = parent->sha1;
    son->sha256 = parent->sha256;
    son->r_flag = parent->r_flag;
}

void recurs(struct forensic *parent){
    struct  dirent *de;
    
    //if dir
    if((parent.last.st_mode & S_IFMT) == S_IFDIR){
        DIR *dr = opendir(fs.name);

        if(parent->r_flag){

        }
        else{
            //opens all files in the current directory 
            while((de = readdir(dr)) != NULL){
                struct forensic son;
                init(&son);
                son.name = (char *) malloc(sizeof(char) * strlen(de->d_name));
                strcpy(son.name, de->d_name);
                son_struct(son, fs);

                recurs(&son);
            }
            
        }
    }
    //if file
    else{
        print_data();
    }
}