//
// Created by zhaogang on 2025-05-24.
//


#include <stdio.h>
#include "cvsdb/cvsdb.h"
#include "cvs/clog.h"

#define db_path "/etc/cvs/cvs.db"
#define log_path "/tmp/cvs.log"


int main(){
    /*初始化cvs_log*/
    int res = cvs_log_init(log_path,CVS_LOG_DEBUG, 1 * 1024 * 10, 5);
    if(res == -1){
        printf("init log failed\n ");
        return -1;
    }

    cvsdb_init(db_path);
    struct CvsPort ports1 = {
            .name = "port1",
            .type = "veth"
    };
    struct CvsPort ports2 = {
            .name = "port2",
            .type = "veth"
    };
    struct CvsPort ports3 = {
            .name = "ens160",
            .type = "dpdk"
    };
    struct CvsPort ports4 = {
            .name = "ens192",
            .type = "dpdk"
    };
    struct CvsPort *ports[] = {
            &ports1,
            &ports2,
            &ports3,
            &ports4

    };
    struct CvsBridge bridge = {
        .name = "br0",
        .port_num = sizeof(ports) / sizeof(ports[0]),
        .port = ports
    };
    cvsdb_add_bridge(&bridge);

    struct CvsFlow flow1 = {
        .bridge = "br0",
        .in_port = "port1",
        .out_port = "port2"
    };
    struct CvsFlow flow2 = {
        .bridge = "br0",
        .in_port = "port2",
        .out_port = "port1"
    };
    struct CvsFlow flow3 = {
            .bridge = "br0",
            .in_port = "ens160",
            .out_port = "ens192"
    };
    struct CvsFlow flow4 = {
            .bridge = "br0",
            .in_port = "ens192",
            .out_port = "ens160\",\n"
                        "            .out_port = \"ens192"
    };

    cvsdb_add_flow(&flow1);
    cvsdb_add_flow(&flow2);
    cvsdb_add_flow(&flow3);
    cvsdb_add_flow(&flow4);

    struct CvsDb *db_ptr = cvsdb_get_db();
    struct cJSON *port_ptr = NULL;
    FOR_EACH_PORT(db_ptr->root, bridge_idx, port_idx, port_ptr)
        LOG_DEBUG("Monitor Port Name: %s",cJSON_GetObjectItem(port_ptr, "name")->valuestring);
    }}


}