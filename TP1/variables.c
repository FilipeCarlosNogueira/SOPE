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

    //number of files
    aux->ndir = 0;
    aux->nfile = 0;
}
