//
// Created by zhaogang on 2025-03-01.
//

#ifndef CVS_CSVDB_H
#define CVS_CSVDB_H
#include <stdlib.h>
#include "cJSON.h"
#include <stdlib.h>
#include "cvs/clog.h"
#include "utils/file.h"

struct CvsDb{
    cJSON *root;
    FILE *fd;
    char *db_path;
    char *data;

};

struct CvsPort {
    char *name;
    char *type;
    char *mac;
    char *ip;
    char *bridge;
};

struct CvsBridge{
    char *name;
    int port_num;
    struct CvsPort **port;
};

struct CvsFlow{
    char *id;
    char *bridge;
    char *in_port;
    char *out_port;
};

int cvsdb_init(char *db_path);
int cvsdb_init_data();
int cvsdb_flush();
int cvsdb_free();

struct CvsDb *cvsdb_get_db();

int cvsdb_add_bridge(struct CvsBridge *bridge);

struct CvsBridge *cvsdb_get_bridge();
int cvsdb_del_bridge(struct CvsBridge *bridge);


int cvsdb_add_port(struct CvsPort *port_name);

struct CvsPort *cvsdb_get_port_list();

int cvsdb_del_port(struct CvsPort *port_name);

int cvsdb_add_flow(struct CvsFlow *flow);


#define FOR_EACH_PORT(db_ptr, bridge_idx, port_idx, port_ptr) \
    cJSON *for_bridges = cJSON_GetObjectItem(db_ptr, "bridges");                                                     \
    for (int bridge_idx = 0; bridge_idx < cJSON_GetArraySize(for_bridges); ++bridge_idx) {                           \
        cJSON *for_ports = cJSON_GetObjectItem(cJSON_GetArrayItem(for_bridges,bridge_idx), "ports");                 \
        for (int port_idx = 0; port_idx < cJSON_GetArraySize(for_ports);++port_idx) {                                \
                            port_ptr = cJSON_GetArrayItem(for_ports, port_idx);

#endif //CVS_CSVDB_H
