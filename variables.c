#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "variables.h"

void init(struct forensic *aux){
    aux->name = NULL;
    aux->output_file = STDOUT_FILENO;
    aux->execution_register = NULL;
    aux->md5 = false;
    aux->sha1 = false;
    aux->sha256 = false;
    aux->r_flag = false;
}