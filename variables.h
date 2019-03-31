#ifndef VARIABLES_H_
#define VARIABLES_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>

struct forensic
{
    struct stat last;
    char *name;
    char *output_file;
    char *execution_register;
    bool md5;
    bool sha1;
    bool sha256;
    bool r_flag;
};

void init(struct forensic *aux);

#endif