#ifndef INTERFACE_H
#define INTERFACE_H

#include "variables.h"
#include "file.h"

void parse_parent_son(struct forensic *son, struct forensic *parent);
int directory_handler(struct forensic *parent, struct dirent *de);
void recurs(struct forensic *parent);

#endif
