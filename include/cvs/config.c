//
// Created by zhaogang on 2025-03-03.
//

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>



// 用于去除首尾空格
char* trim(char* str) {
    while (isspace((unsigned char)*str)) str++;  // 去掉前面的空格
    if (*str == 0) return str;  // 如果字符串为空，直接返回

    // 去掉尾部空格
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';

    return str;
}

int cvs_config_init(const char *config_path, struct CvsConfig **cvs_config) {
    if (config_path == NULL) return -1;

    struct CvsConfig *cvs_c = malloc(sizeof(struct CvsConfig));
    cvs_c->default_config = malloc(sizeof(struct DefaultConfig));

    FILE *fp = fopen(config_path, "r");
    if (fp == NULL) {
        printf("open config file failed\n");
        return -1;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, fp)) != -1) {
        printf("%s", line);

        // 跳过注释和空行
        if (*line == '#' || *line == '\n' || *line == '\r' || *line == '[') continue;

        // 去除首尾空格
        line = trim(line);

        // 查找分隔符的位置（= 或 :）
        char *delimiter = strchr(line, '=');
        if (!delimiter) delimiter = strchr(line, ':');  // 支持冒号分隔符

        if (delimiter) {
            // 分隔符前是键，分隔符后是值
            *delimiter = '\0';  // 将分隔符转换为字符串结束符
            char *key = trim(line);
            char *value = trim(delimiter + 1);

            // 根据键设置对应值
            if (strcmp(key, "db_path") == 0) {
                cvs_c->default_config->db_path = strdup(value);
            } else if (strcmp(key, "log_path") == 0) {
                cvs_c->default_config->log_path = strdup(value);
            } else if (strcmp(key, "unix_path") == 0) {
                cvs_c->default_config->unix_path = strdup(value);
            } else if (strcmp(key, "tcp_path") == 0) {
                cvs_c->default_config->tcp_path = strdup(value);
            } else if (strcmp(key, "debug") == 0) {
                cvs_c->default_config->debug = (strcmp(value, "true") == 0);
            }
        }
    }

    free(line);
    fclose(fp);

    *cvs_config = cvs_c;
    return 1;
}
struct DefaultConfig *cvs_config_get_default(struct CvsConfig *cvs_config){
    if(cvs_config == NULL){
        return NULL;
    }
    return cvs_config->default_config;
}