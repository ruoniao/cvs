//
// Created by zhaogang on 2025-03-01.
//

#ifndef CVS_CSVDB_H
#define CVS_CSVDB_H

#include "cJSON.h"

struct CvsDb{
    cJSON *root;
    char *db_path;
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
    struct CvsPort *port;
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

int cvsdb_add_bridge(struct CvsBridge *bridge);
struct CvsDb *cvsdb_get_bridge(struct CvsBridge *bridge);
int cvsdb_del_bridge(struct CvsBridge *bridge);


int cvsdb_add_port(struct CvsDb bridge_nam, struct CvsPort *port_name);
int cvsdb_del_port(struct CvsDb bridge_name, struct CvsPort *port_name);

int cvsdb_add_flow(struct CvsDb bridge_name, const char *flow_name);


#endif //CVS_CSVDB_H
