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

// 创建文件
int file_create(const char *path);

// 读取文件全部内容
char *file_read_all(FILE *fp);

#endif //CVS_FILE_H
