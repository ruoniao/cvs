//
// Created by zhaogang on 2025-03-16.
//

#include "utils/file.h"


int file_create(const char *db_path){
    // 使用 stat 检查文件夹是否存在
    char dir_path[256]; // 假设足够大，存储目录路径
    struct stat st = {0};
    // 查找最后一个路径分隔符
    const char* last_slash = strrchr(db_path, '/');
    if (last_slash == NULL) {
        LOG_DEBUG("路径中没有分隔符\n");
        strcpy(dir_path, "."); // 默认当前目录
    } else {
        // 计算目录路径长度
        size_t len = last_slash - db_path;
        strncpy(dir_path, db_path, len);
        dir_path[len] = '\0'; // 添加字符串结束符
    }
    if (stat(dir_path, &st) == -1) {
        // 文件夹不存在，创建它
        if (mkdir(dir_path, 0755) == -1) {
            perror("创建文件夹失败");
            return -1; // 创建失败
        }
        LOG_DEBUG("文件夹 %s 创建成功\n", dir_path);
    } else {
        LOG_DEBUG("文件夹 %s 已存在\n", dir_path);
    }
    FILE *fp = fopen(db_path, "w");
    if (fp == NULL) {
        LOG_ERROR("创建文件 %s 失败\n",db_path);
        return -1;
    }
    fclose(fp);
    LOG_DEBUG("创建文件 %s 成功\n",db_path);
    return 1;
}