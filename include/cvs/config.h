//
// Created by zhaogang on 2025-03-03.
//

#ifndef CVS_CONFIG_H
#define CVS_CONFIG_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

struct DefaultConfig {
    bool debug;
    char *db_path;
    char *log_path;
};

struct DaemonConfig {
    int server_mode;
    char *host;
    int port;
    char *unix_path;
    char *tcp_path;
};
struct CvsConfig {
    struct DefaultConfig *default_config;
    struct DaemonConfig *daemon_config;
};


int cvs_config_init(const char *config_path,struct CvsConfig **cvs_config);
int cvs_config_free(struct CvsConfig *cvs_config);

/*获取配置函数*/
struct DefaultConfig *cvs_config_get_default(struct CvsConfig *cvs_config);

#endif //CVS_CONFIG_H
