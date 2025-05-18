//
// Created by zhaogang on 2025-03-16.
//

#ifndef CVS_FILE_H
#define CVS_FILE_H
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cvs/clog.h"

int file_create(const char *path);


#endif //CVS_FILE_H
