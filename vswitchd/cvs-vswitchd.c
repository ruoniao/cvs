//
// Created by zhaogang on 2025-03-01.
//
#include <stdio.h>
#include "cvs/clog.h"
#include "cvs/config.h"
#include "cvs/cmd.h"
#include "cvsdb/cvsdb.h"

struct CvsConfig *cvs_config;
struct DefaultConfig *default_config;


int main(int argc, char *argv[]){

    /* 解析argc命令参数 */
    cmd_parse_option(argc, argv);

    /*初始化cvs_config*/
    cvs_config_init(cmd_get_option("config"),&cvs_config);
    default_config = cvs_config_get_default(cvs_config);

    /*初始化cvs_log*/
    int res = cvs_log_init(default_config->log_path, default_config->debug, 1 * 1024 * 10, 5);
    if(res == -1){
        printf("init log failed\n");
        return -1;
    }

    /*初始化cvsdb*/
    cvsdb_init(default_config->db_path);


    //cvsdb_add_bridge();
    /*初始化cvs-vswitchd 支持启动unix/tcp 与csv-ctl 通讯*/

    /*初始化cvs-vswitchd的网桥*/

    /*初始化cvs-vswitchd的net管理，支持poll/epoll/dpdk模式收发*/

    /*初始化cvs-vswitchd的流表*/

cleanup:
    free(cvs_config);
    cvsdb_free();
//    cvs_config_cleanup();
//    cvs_log_cleanup();
    return 1 ;
}
