#ifndef INTERFACE_H
#define INTERFACE_H

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

int parse_parent_son(struct forensic *son, struct forensic *parent);
void recurs(struct forensic *parent);

#endif