//
// Created by zhaogang on 2025-03-03.
//


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "cvs/config.h"
#include "cvs/types.h"


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
    cvs_c->default_config = calloc(1, sizeof(struct DefaultConfig));
    cvs_c->daemon_config = calloc(1, sizeof(struct DaemonConfig));

    FILE *fp = fopen(config_path, "r");
    if (fp == NULL) {
        printf("open config file failed\n");
        return -1;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char current_section[64] = {0};

    while ((read = getline(&line, &len, fp)) != -1) {
        // 去除首尾空白字符
        char *line_trimmed = trim(line); // 不覆盖 line

        // 跳过空行和注释
        if (*line_trimmed == '\0' || *line_trimmed == '#' || *line_trimmed == ';') continue;

        // 检查是否是分组
        if (line_trimmed[0] == '[' && line_trimmed[strlen(line_trimmed) - 1] == ']') {
            strncpy(current_section, line_trimmed + 1, strlen(line_trimmed) - 2);
            current_section[strlen(line_trimmed) - 2] = '\0';
            continue;
        }

        // 查找键值分隔符
        char *delimiter = strchr(line_trimmed, '=');
        if (!delimiter) delimiter = strchr(line_trimmed, ':');
        if (!delimiter) continue;

        *delimiter = '\0';
        char *key = trim(line_trimmed);
        char *value = trim(delimiter + 1);

        if (strcmp(current_section, "default") == 0) {
            if (strcmp(key, "db_path") == 0)
                cvs_c->default_config->db_path = strdup(value);
            else if (strcmp(key, "log_path") == 0)
                cvs_c->default_config->log_path = strdup(value);
            else if (strcmp(key, "debug") == 0)
                cvs_c->default_config->debug = strcmp(value, "true") == 0;
        } else if (strcmp(current_section, "daemon") == 0) {
            if (strcmp(key, "unix_path") == 0)
                cvs_c->daemon_config->unix_path = strdup(value);
            else if (strcmp(key, "tcp_path") == 0)
                cvs_c->daemon_config->tcp_path = strdup(value);
            else if (strcmp(key, "host") == 0)
                cvs_c->daemon_config->host = strdup(value);
            else if (strcmp(key, "port") == 0)
                cvs_c->daemon_config->port = atoi(value);
            else if (strcmp(key, "server_mode") == 0) {
                if (strcmp(value, "tcp") == 0)
                    cvs_c->daemon_config->server_mode = MODE_TCP; // TCP
                else if (strcmp(value, "unix") == 0)
                    cvs_c->daemon_config->server_mode = MODE_UNIX; // UNIX
            }
        }
        // 可添加其他分组
    }
    if(line != NULL)
        free(line);
    fclose(fp);

    *cvs_config = cvs_c;
    return 0;
}

struct DefaultConfig *cvs_config_get_default(struct CvsConfig *cvs_config){
    if(cvs_config == NULL){
        return NULL;
    }
    return cvs_config->default_config;
}