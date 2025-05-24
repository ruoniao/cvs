//
// Created by zhaogang on 2025-03-01.
//

#include "cvs/clog.h"


static Logger logger = {
    .bash_path = "/var/log/cvs/cvs.log",
    .level = CVS_LOG_INFO,
    .size = 1024 * 1024 * 10,
    .max_files = 5,
    .fp = NULL,
    .mutex = PTHREAD_MUTEX_INITIALIZER
};

/*
 * 初始化日志
 */
int cvs_log_init(char *log_path, LogLevel level, size_t size, int max_files){
    if(log_path != NULL){
        logger.bash_path = log_path;
    }
    logger.level = level;
    logger.size = size;
    logger.max_files = max_files;
    logger.fp = fopen(logger.bash_path, "a+");
    if(logger.fp == NULL){
        printf("open log file failed: %s\n", strerror(errno));
        return -1;
    }
    return 1;
}

/*
 * 分割日志
 */
static void rotate_log(){
    char new_path[256];
    char old_path[256];
    char time_str[32];
    get_time_str(time_str, sizeof(time_str));
    fclose(logger.fp);
    for(int i = logger.max_files - 1; i > 0; i--){
        snprintf(new_path, sizeof(new_path), "%s.%d", logger.bash_path, i);
        snprintf(old_path, sizeof(old_path), "%s.%d", logger.bash_path, i - 1);
        rename(old_path, new_path);
    }
    snprintf(new_path, sizeof(new_path), "%s.0", logger.bash_path);
    rename(logger.bash_path, new_path);
    logger.fp = fopen(logger.bash_path, "a+");
    if(logger.fp == NULL){
        return;
    }
}

/*
 * 核心日志函数，写日志
 */
void log_write(LogLevel level,const char *file, int line, const char *fmt, ...){
    if (level != CVS_LOG_ERROR && level > logger.level || !logger.fp){
        return;
    }
    pthread_mutex_lock(&logger.mutex);
    /*检查文件大小*/
    fseek(logger.fp, 0, SEEK_END);
    /*获取文件大小*/
    long file_size = ftell(logger.fp);
    if(file_size >= logger.size){
        rotate_log();
    }

    /*获取时间*/
    char time_str[32];
    get_time_str(time_str, sizeof(time_str));

    /*格式化日志头   [时间] [文件:行] [日志级别] */
    const char *level_str = "";
    switch (level) {
        case CVS_LOG_INFO:
            level_str = "INFO";
            break;
        case CVS_LOG_DEBUG:
            level_str = "DEBUG";
            break;
        case CVS_LOG_ERROR:
            level_str = "ERROR";
            break;
        default:
            break;
    }
    fprintf(logger.fp, "[%s] [%s:%d] [%s] ", time_str, file, line, level_str);
    va_list args;
    va_list args_copy;
    va_start(args, fmt);
    va_copy(args_copy, args);
    // 写入日志内容
    vfprintf(logger.fp, fmt, args);
    fprintf(logger.fp, "\n");
    // 打印到标准输出
    printf("[%s] [%s:%d] [%s]", time_str, file, line, level_str);
    //  知识点：printf 用于直接传参，vprintf 用于变参封装后传递（va_list）的场景。不要混用。
    vprintf( fmt, args_copy);
    printf("\n");
    va_end(args);
    va_end(args_copy);

    fflush(logger.fp);
    pthread_mutex_unlock(&logger.mutex);
}

/*
 * 获取时间字符串
 */
static void get_time_str(char *buf, size_t len){
    time_t t;
    struct tm *tm;
    t = time(NULL);
    tm = localtime(&t);
    strftime(buf, len, "%Y-%m-%d %H:%M:%S", tm);

}

/*
 * 释放日志；TODO 啥时候用，怎么用？不释放会怎么样？
 */
void cvs_log_free(){
    pthread_mutex_lock(&logger.mutex);
    if(logger.fp){
        fclose(logger.fp);
    }
    pthread_mutex_unlock(&logger.mutex);
}

