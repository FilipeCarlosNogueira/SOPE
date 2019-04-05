#ifndef VARIABLES_H_
#define VARIABLES_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/times.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

struct forensic
{
        //stat struct
        struct stat last;

        //time the process took.
        struct tms time;
        clock_t start;

        //parent pid
        pid_t pid;

        //strings
        char name[100];

        //file descriptors
        int output_file;
        int execution_register;

        //algorithm flags
        bool md5;
        bool sha1;
        bool sha256;

        //recursive flag
        bool r_flag;

        //number of files
        int ndir;
        int nfile;
};

void init(struct forensic *aux);
extern int n_directories;
extern int n_files;

#endif
