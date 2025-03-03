//
// Created by zhaogang on 2025-03-03.
//

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>


char *trim(char *str) {
    // 去掉首部空白字符
    while (isspace((unsigned char)*str)) {
        str++;
    }
    // 去掉尾部空白字符
    if (*str) {
        char *end = str + strlen(str) - 1;
        while (end > str && (isspace((unsigned char)*end) || *end == '"')) {
            *end-- = '\0';
        }
    }
    // 去掉首部引号
    if (*str == '"') {
        str++;
    }
    return str;
}

int cvs_config_init(const char *config_path){
    if(config_path == NULL){
        return -1;
    }
    FILE *fp = fopen(config_path, "r");
    if(fp == NULL){
        return -1;
    }
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while((read = getline(&line, &len, fp)) != -1){
        printf("%s", line);
        if(*line == '#' || *line == '\n'|| *line == '\r'|| *line == '['){
            continue;
        }
        if(strncmp(line, "db_path", 7) == 0) {
            cvs_config.default_config.db_path = strdup(trim(line + 8));
        } else if(strncmp(line, "log_path", 8) == 0) {
            cvs_config.default_config.log_path = strdup(trim(line + 9));
        } else if(strncmp(line, "unix_path", 9) == 0) {
            cvs_config.default_config.unix_path = strdup(trim(line + 10));
        } else if(strncmp(line, "tcp_path", 8) == 0) {
            cvs_config.default_config.tcp_path = strdup(trim(line + 9));
        } else if (strncmp(line, "debug", 5) == 0) {
            if (strncmp(line + 6, "true", 4) == 0) {
                cvs_config.default_config.debug = 1;
            } else {
                cvs_config.default_config.debug = 0;
            }
        }
    }
    free(line);
    fclose(fp);
    return 1;
}
