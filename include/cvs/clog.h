//
// Created by zhaogang on 2025-03-03.
//

#ifndef CVS_CLOG_H
#define CVS_CLOG_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

typedef enum {
    CVS_LOG_INFO,
    CVS_LOG_DEBUG,
    CVS_LOG_ERROR
} LogLevel;


typedef struct {
    char *bash_path;
    LogLevel level;
    size_t size;
    int max_files;
    FILE *fp;
    pthread_mutex_t mutex;
}Logger;

static Logger logger;

/*内部函数声明*/
static void rotate_log();
static void get_time_str(char *buf, size_t len);


/*对外接口*/
int cvs_log_init(char *log_path, LogLevel level, size_t size, int max_files);
void log_write(LogLevel level,const char *file, int line, const char *fmt, ...);

#define LOG_INFO(fmt, ...) log_write(CVS_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) log_write(CVS_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) log_write(CVS_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif //CVS_CLOG_H
