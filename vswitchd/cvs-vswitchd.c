//
// Created by zhaogang on 2025-03-01.
//
#include <stdio.h>
#include "cvs/clog.c"
#include "cvs/config.c"
#include "cvs/cmd.c"

int main(int argc, char *argv[]){

    cmd_parse_option(argc, argv);

    cvs_config_init(cvs_cmd_config.config);

    int res = cvs_log_init(cvs_config.default_config.log_path, cvs_config.default_config.debug, 1 * 1024 * 10, 5);
    if(res == -1){
        printf("init log failed\n");
        return -1;
    }


    LOG_INFO("Hello, LOG_INFO!");
    LOG_DEBUG("Hello, LOG_DEBUG!");
    LOG_ERROR("Hello, LOG_ERROR!");
    /*解析命令行args --db xx.json*/
    //parse_cmd(argc, argv);

    /*初始化cvsdb*/

    /*初始化cvs-vswitchd 支持启动unix/tcp 与csv-ctl 通讯*/

    /*初始化cvs-vswitchd的网桥*/

    /*初始化cvs-vswitchd的net管理，支持poll/epoll/dpdk模式收发*/

    /*初始化cvs-vswitchd的流表*/
    return 1 ;
}
