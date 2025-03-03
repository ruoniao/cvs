//
// Created by zhaogang on 2025-03-03.
//

#ifndef CVS_CMD_H
#define CVS_CMD_H

#include <stdio.h>
#include <string.h>
#include "config.h"

struct CvsCmdConfig {
    char *config;
    char *db;
    char *log;
    char *unix_path;
    char *tcp;
};

struct CvsCmdConfig cvs_cmd_config;
int cmd_parse_option(int argc, char *argv[]);

#endif //CVS_CMD_H
