//
// Created by zhaogang on 2025-05-24.
//


#include <stdio.h>
#include "cvsdb/cvsdb.h"
#include "cvs/clog.h"

#define db_path "/tmp/cvsdb1.json"
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
    };
    struct CvsPort ports2 = {
            .name = "port2",
    };
    struct CvsPort *ports[] = {
            &ports1,
            &ports2,

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

    cvsdb_add_flow(&flow1);
    cvsdb_add_flow(&flow2);
}