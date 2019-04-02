#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "variables.h"

void init(struct forensic *aux){
    //strings
    strcpy(aux->name, "");

    //parent pid
    aux->pid = getpid();

    //file descriptors
    aux->output_file = STDOUT_FILENO;
    aux->execution_register = -1;

    //algorithm flags
    aux->md5 = false;
    aux->sha1 = false;
    aux->sha256 = false;

    //recursive flag
    aux->r_flag = false;
}