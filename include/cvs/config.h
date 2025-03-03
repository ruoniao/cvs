//
// Created by zhaogang on 2025-03-03.
//

#ifndef CVS_CONFIG_H
#define CVS_CONFIG_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

struct CvsConfig {
    struct default_config {
        char *db_path;
        char *log_path;
        char *unix_path;
        char *tcp_path;
        bool debug;
    } default_config;
};

struct CvsConfig cvs_config = {
        .default_config = {
                .db_path = "/var/log/cvs/cvs.db",
                .log_path = "/var/log/cvs/cvs.log",
                .unix_path = "/var/log/cvs/cvs.sock",
                .tcp_path = "",
                .debug = false
        }
};

int cvs_config_init(const char *config_path);

#endif //CVS_CONFIG_H
