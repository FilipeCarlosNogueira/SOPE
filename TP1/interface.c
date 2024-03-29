#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <sys/wait.h>

#include "variables.h"
#include "file.h"
#include "interface.h"

int ndir;
int nfile;

void sig_handler(int sig)
{
        if(sig == SIGUSR1) {
                ndir++;
                printf("New directory: %d/%d directories/files at this time\n", ndir, nfile);
        }
        if(sig == SIGUSR2)
        {
                nfile++;
        }
}

//parses all perminent information to the son struct
void parse_parent_son(struct forensic *son, struct forensic *parent){

        //output file
        son->output_file = parent->output_file;

        //execution register
        son->execution_register = parent->execution_register;

        //flags
        son->md5 = parent->md5;
        son->sha1 = parent->sha1;
        son->sha256 = parent->sha256;
        son->r_flag = parent->r_flag;

        //file or dir stat
        if(lstat(son->name, &son->last) != 0) {
                perror("stat son");
                exit(1);
        }

        //saving the first parent pid
        son->pid = parent->pid;

        //saving the moment of the first process
        son->time = parent->time;
        son->start = parent->start;

        //saving number of files
        son->ndir = parent->ndir;
        son->nfile = parent->nfile;
}

int directory_handler(struct forensic *parent, struct dirent *de){
        int status;
        char aux[20];

        //if the element in the parent directory starts with '.' or ".." it ignores
        if(de->d_name[0] == '.') {
                return 1;
        }

        struct forensic son;
        //inicializes son's variables.
        init(&son);

        //saves name
        sprintf(son.name, "%s", de->d_name);

        //parses all perminent information to the son struct
        parse_parent_son(&son, parent);

        //if the element in consideration is an directory
        if((son.last.st_mode & S_IFMT) == S_IFDIR) {
                //the user wants recursive
                if(parent->r_flag) {
                        // //if user specified the execution register
                        // if(son.execution_register != -1){
                        //     strcpy(h_aux, "COMAND forensic -r ");
                        //     strcat(h_aux, "./");
                        //     strcat(h_aux, son.name);
                        //     strcat(h_aux, "\n");

                        //     write(son.execution_register, h_aux, strlen(h_aux)*sizeof(char));
                        // }

                        pid_t pid;
                        //it creates a seperate process to compute it.
                        if((pid = fork()) == 0) {
                                recurs(&son);
                                exit(0);
                        }
                        pid = wait(&status);
                }
                //if the user doen't want recursive then it shows nothing
        }
        //if its a file, it print automatically
        else if(((son.last.st_mode & S_IFMT) == S_IFREG)) {
                if(son.output_file != 1) {
                        ndir = son.ndir;
                        raise(SIGUSR2);
                        if(son.execution_register != -1) {
                                strcpy(aux, "SIGUSR2\n");
                                write(son.execution_register, aux, strlen(aux));
                        }
                }
                //if its a subfolder it passes the subfolder name
                if(son.pid != getppid())
                        print_data(&son, NULL);
                //if its the first folder then no name is passed
                else
                        print_data(&son, parent->name);
        }
        return 0;
}

void recurs(struct forensic *parent){
        //if dir
        char aux[20];

        struct sigaction dir;

        dir.sa_handler=sig_handler;
        sigemptyset(&dir.sa_mask);
        dir.sa_flags=SA_RESTART;

        sigaction(SIGUSR1,&dir,NULL);

        struct sigaction file;

        file.sa_handler=sig_handler;
        sigemptyset(&file.sa_mask);
        file.sa_flags=SA_RESTART;

        sigaction(SIGUSR2,&file,NULL);

        if((parent->last.st_mode & S_IFMT) == S_IFDIR) {
                if(parent->output_file != 1) {
                        parent->ndir++;
                        ndir = parent->ndir;
                        raise(SIGUSR1);
                        if(parent->execution_register != -1) {
                                strcpy(aux, "SIGUSR1\n");
                                write(parent->execution_register, aux, strlen(aux));
                        }
                }
                //opens parent directory
                DIR *dr = opendir(parent->name);
                if(dr == NULL) {
                        perror("dr");
                        exit(1);
                }
                seekdir(dr, 0);

                //changes to parent directory
                chdir(parent->name);

                struct dirent *de;
                //opens all files in the current directory
                while((de = readdir(dr)) != NULL) {

                        //if the element in the parent directory starts with '.' or ".." it ignores
                        if(directory_handler(parent, de))
                                continue;
                }

                //closes parent directory
                closedir(dr);
        }
        //if file
        else{
                if(parent->output_file != 1) {
                        parent->nfile++;
                        nfile = parent->nfile;
                        raise(SIGUSR2);
                        if(parent->execution_register != -1) {
                                strcpy(aux, "SIGUSR2\n");
                                write(parent->execution_register, aux, strlen(aux));
                        }
                }
                print_data(parent, NULL);
        }
}
