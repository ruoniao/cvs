//
// Created by zhaogang on 2025-03-01.
//
#include <stdio.h>
#include "cvs/clog.h"
#include "cvs/config.h"
#include "cvs/cmd.h"
#include "server/server-daemon.h"

#include "cvsdb/cvsdb.h"
#include "netdev/netdev.h"
#include <unistd.h>



struct CvsConfig *cvs_config;
struct DefaultConfig *default_config;


int main(int argc, char *argv[]){

    bool exiting = false;
    /* 解析argc命令参数 */
    cmd_parse_option(argc, argv);

    /*初始化cvs_config*/
    cvs_config_init(cmd_get_option("config"),&cvs_config);
    default_config = cvs_config_get_default(cvs_config);

    /*初始化cvs_log*/
    int res = cvs_log_init(default_config->log_path, default_config->debug, 1 * 1024 * 10, 5);
    if(res == -1){
        printf("init log failed\n ");
        return -1;
    }

    /*初始化cvsdb*/
    cvsdb_init(default_config->db_path);

    /* 启动daemon server*/
    struct ServerConfig *daemon_config = server_config_init(cvs_config->daemon_config->server_mode,
            cvs_config->daemon_config->host, cvs_config->daemon_config->port,
            cvs_config->daemon_config->tcp_path);
    cvs_server_start(daemon_config);

    /* 初始化网卡驱动类 */
    //netdev_init();
    while (!exiting){
        /* 处理命令行参数 */
        //cvs_cmd_process();
        /* 处理信号 */
        //cvs_signal_process();
        /* 处理网卡事件 */
        //cvs_netdev_process();
        /* 处理流表事件 */
        //cvs_flow_process();
        struct CvsDb *db_ptr = cvsdb_get_db();
        struct cJSON *port_ptr = NULL;
        FOR_EACH_PORT(db_ptr->root, bridge_idx, port_idx, port_ptr)
           LOG_DEBUG("Monitor Port Name: %s",cJSON_GetObjectItem(port_ptr, "name")->valuestring);
        }}

        netdev_run();
        //sleep(1);
    }

cleanup:
    free(cvs_config);
    cvsdb_free();
    return 1 ;
}
