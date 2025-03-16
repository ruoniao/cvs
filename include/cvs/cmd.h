//
// Created by zhaogang on 2025-03-03.
//

#ifndef CVS_CMD_H
#define CVS_CMD_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct CvsCmdConfig {
    char *config;
    char *db;
    char *log;
    char *unix_path;
    char *tcp;
};

int cmd_parse_option(int argc, char *argv[]);
char *cmd_get_option(const char *key);

#endif //CVS_CMD_H
