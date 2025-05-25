//
// Created by zhaogang on 2025-03-01.
//

#include <stdlib.h>
#include "cvsdb/cvsdb.h"
#include "cvs/clog.h"
#include "utils/file.h"

struct CvsDb *db;


int cvsdb_init(char *db_path){
    if (db_path == NULL) {
        LOG_ERROR("db_path is NULL\n");
        return -1;
    }
    FILE *fp = fopen(db_path, "r");
    if (fp == NULL) {
        LOG_ERROR("db file not exist,will be create\n");
        int res = file_create(db_path);
        if(res == -1) {
            return -1;
        }
    }
    db = (struct CvsDb *)malloc(sizeof(struct CvsDb));
    if(db == NULL){
        return -1;
    }
    char *data = file_read_all(fp);
    db->db_path = db_path;
    db->data = data;
    db->fd = fp;
    db->root = cJSON_Parse(data);
    if(db->root == NULL){
        db->root = cJSON_CreateObject();
        cvsdb_init_data();
    }
    return 1 ;
}

int cvsdb_init_data(){
    cJSON *bridges = cJSON_GetObjectItem(db->root, "bridges");
    if(bridges == NULL){
        bridges = cJSON_CreateArray();
        cJSON_AddItemToObject(db->root, "bridges", bridges);
    }
    /*
    赶进度，暂时不关注这个
    cJSON *ports = cJSON_GetObjectItem(db->root, "ports");
    if(ports == NULL){
        ports = cJSON_CreateArray();
        cJSON_AddItemToObject(db->root, "ports", ports);
    }*/
    cJSON *flows = cJSON_GetObjectItem(db->root, "flows");
    if(flows == NULL){
        flows = cJSON_CreateArray();
        cJSON_AddItemToObject(db->root, "flows", flows);
    }

    //cJSON_AddItemToObject(bridges, "name", cJSON_CreateString("br-int"));
    cvsdb_flush();
    return 1;
}

int cvsdb_flush(){
    if(db->root == NULL){
        return -1;
    }
    // 将 JSON 对象转为字符串
    char *json_string = cJSON_Print(db->root);  // 生成格式化的字符串
    if (json_string == NULL) {
        LOG_ERROR("Error printing JSON to string\n");
        return -1;
    }

    // 打开文件并写入
    FILE *fp = fopen(db->db_path, "w");  // 假设文件名是 config.json
    if (fp == NULL) {
        LOG_ERROR("Error opening file\n");
        free(json_string);
        return -1;
    }

    fwrite(json_string, 1, strlen(json_string), fp);  // 将字符串写入文件
    fclose(fp);

    // 释放 JSON 字符串内存
    free(json_string);
    LOG_INFO("Flush db_pathto file\n");
    return 1;

}
int cvsdb_free(){
    //cJSON_Delete(db->root);
    free(db->fd);
    free(db->data);
    free(db);
    return 1;
}

struct CvsDb *cvsdb_get_db(){
    if(db == NULL){
        LOG_ERROR("db is NULL\n");
        return NULL;
    }
    return db;
}

int cvsdb_add_bridge(struct CvsBridge *bridge){
    cJSON *bridges = cJSON_GetObjectItem(db->root, "bridges");
    if(bridges == NULL){
        bridges = cJSON_CreateArray();
        cJSON_AddItemToObject(db->root, "bridges", bridges);
        //cJSON_AddArrayToObject(bridges,NULL);
    }

    cJSON *bridge_obj = cJSON_CreateObject();
    int bridges_len = cJSON_GetArraySize(bridges);
    for (int i=0;i<=bridges_len;i++) {
        cJSON *item = cJSON_GetArrayItem(bridges, i);
        if (item != NULL && cJSON_IsObject(item)) {
            const char *name = cJSON_GetObjectItem(item, "name")->valuestring;
            if (strcmp(name, bridge->name) == 0) {
                LOG_ERROR("Bridge %s already exists\n", bridge->name);
                return -1; // Bridge already exists
            }
        }
    }
    cJSON_AddItemToObject(bridge_obj, "name", cJSON_CreateString(bridge->name));
    cJSON *ports = cJSON_CreateArray();
    if (bridge->port != NULL && bridge->port_num > 0) {
        for (int i = 0; i < bridge->port_num; i++) {
            cJSON *port_obj = cJSON_CreateObject();
            cJSON_AddItemToObject(port_obj, "name", cJSON_CreateString(bridge->port[i]->name));
            //cJSON_AddItemToObject(port_obj, "type", cJSON_CreateString(bridge->port[i].type));
            //cJSON_AddItemToObject(port_obj, "mac", cJSON_CreateString(bridge->port[i].mac));
            //cJSON_AddItemToObject(port_obj, "ip", cJSON_CreateString(bridge->port[i].ip));
            cJSON_AddItemToArray(ports, port_obj);
        }
        printf("Add bridge %s with %d ports: %s\n", bridge->name, bridge->port_num,
                  cJSON_Print(ports));
        cJSON_AddItemToObject(bridge_obj, "ports", ports);
    }

    cJSON_AddItemToArray(bridges, bridge_obj);

    cvsdb_flush();
    return 1;
}
int cvsdb_update_port(struct CvsPort *port){
    struct cJSON *port_ptr = NULL;
    FOR_EACH_PORT(db->root, bridge_idx, port_idx, port_ptr)
        if (strcmp(cJSON_GetObjectItem(port_ptr, "name")->valuestring, port->name) == 0) {
            // 更新端口信息
            if(cJSON_GetObjectItem(port_ptr, "ifindex") == NULL ){
                cJSON_AddItemToObject(port_ptr, "ifindex", cJSON_CreateNumber(port->ifindex));
            } else {
                cJSON_ReplaceItemInObject(port_ptr, "ifindex", cJSON_CreateNumber(port->ifindex));
            }
            if(cJSON_GetObjectItem(port_ptr, "is_up") == NULL ){
                cJSON_AddItemToObject(port_ptr, "is_up", cJSON_CreateBool(port->is_up));
            } else {
                cJSON_ReplaceItemInObject(port_ptr, "is_up", cJSON_CreateBool(port->is_up));
            }
            if(cJSON_GetObjectItem(port_ptr, "is_running") == NULL ){
                cJSON_AddItemToObject(port_ptr, "is_running", cJSON_CreateBool(port->is_running));
            } else {
                cJSON_ReplaceItemInObject(port_ptr, "is_running", cJSON_CreateBool(port->is_running));
            }
            cvsdb_flush();
            LOG_INFO("Updated port %s in bridge %s\n", port->name, port->bridge);
            return 1;
        }
    }}
}



int cvsdb_add_flow(struct CvsFlow *flow){
    cJSON *flows = cJSON_GetObjectItem(db->root, "flows");
    if(flows == NULL){
        flows = cJSON_CreateArray();
        cJSON_AddItemToObject(db->root, "flows", flows);
        //cJSON_AddArrayToObject(bridges,NULL);
    }
    char id_buf[256];  // 注意分配足够的空间
    snprintf(id_buf, sizeof(id_buf), "%s:%s:%s", flow->bridge, flow->in_port, flow->out_port);

    int flows_len = cJSON_GetArraySize(flows);
    for (int i = 0;i < flows_len;i++) {
        cJSON *item = cJSON_GetArrayItem(flows, i);
        if (item != NULL && cJSON_IsObject(item)) {
            const char *id = cJSON_GetObjectItem(item, "id")->valuestring;

            if (strcmp(id, id_buf) == 0) {
                LOG_ERROR("Flow %s already exists\n", flow->id);
                return -1; // Flow already exists
            }
        }
    }
    cJSON *flow_obj = cJSON_CreateObject();
    if (flow->bridge == NULL || flow->in_port == NULL || flow->out_port == NULL) {
        LOG_ERROR("Flow fields cannot be NULL\n");
        return -1; // Invalid flow data
    }
    cJSON_AddItemToObject(flow_obj, "bridge", cJSON_CreateString(flow->bridge));
    cJSON_AddItemToObject(flow_obj, "in_port", cJSON_CreateString(flow->in_port));
    cJSON_AddItemToObject(flow_obj, "out_port", cJSON_CreateString(flow->out_port));

    cJSON_AddItemToObject(flow_obj, "id", cJSON_CreateString(id_buf));

    cJSON_AddItemToArray(flows, flow_obj);
    cvsdb_flush();

    LOG_INFO("Added flow %s with bridge %s, in_port %s, out_port %s\n", id_buf, flow->bridge, flow->in_port, flow->out_port);
    return 1;
}