//
// Created by zhaogang on 2025-03-01.
//

#include <stdlib.h>
#include "cvsdb.h"
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
    db->db_path = db_path;
    db->root = cJSON_Parse(db_path);
    if(db->root == NULL){
        db->root = cJSON_CreateObject();
        cvsdb_init_data();
    }
    return 1 ;
}

int cvsdb_init_data(){
    cJSON *bridges = cJSON_CreateObject();
    cJSON_AddItemToObject(db->root, "bridges", bridges);
    cJSON *ports = cJSON_CreateObject();
    cJSON_AddItemToObject(db->root, "ports", ports);

    cJSON_AddItemToObject(bridges, "name", cJSON_CreateString("br-int"));

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
    free(db);
    return 1;
}


int cvsdb_add_bridge(struct CvsBridge *bridge){
    cJSON *bridges = cJSON_GetObjectItem(db->root, "bridges");
    if(bridges == NULL){
        bridges = cJSON_CreateArray();
        cJSON_AddItemToObject(db->root, "bridges", bridges);
    }

    cJSON *bridge_obj = cJSON_CreateObject();
    cJSON_AddItemToObject(bridge_obj, "name", cJSON_CreateString(bridge->name));
    cJSON_AddItemToObject(bridge_obj, "port", cJSON_CreateString(bridge->port->name));
    cJSON_AddItemToArray(bridges, bridge_obj);

    cvsdb_flush();
    return 1;
}
