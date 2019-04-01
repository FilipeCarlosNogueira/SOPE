#ifndef FILE_H
#define FILE_H

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

char *file_type();
char *algorithm(char *algm);
void print_data(struct forensic *new, char *subfolder);


#endif
