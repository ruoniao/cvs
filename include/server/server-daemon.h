//
// Created by zhaogang on 2025-05-17.
//

#ifndef CVS_SERVER_H
#define CVS_SERVER_H

#include "cvs/types.h"

struct ServerConfig {
    enum ServerMode mode;
    char *host;
    int port;
    char *socket_path;
};

struct ServerConfig *server_config_init(enum ServerMode mode, const char *host, int port, const char *socket_path);

void server_config_free(struct ServerConfig *config);

int cvs_server_start(struct ServerConfig *config);
#endif //CVS_SERVER_H
