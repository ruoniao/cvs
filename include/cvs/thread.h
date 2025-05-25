//
// Created by zhaogang on 2025-04-23.
//

#ifndef CVS_THREAD_H
#define CVS_THREAD_H
#include <stdatomic.h>
#include <stdbool.h>
#include "pthread.h"
#include "compiler.h"

struct cvsthread_once {
    int done;
    pthread_mutex_t mutex;
};

#define CVS_THREAD_ONCE_INIT {0, PTHREAD_MUTEX_INITIALIZER}
#define CVS_THREAD_ONCE_NL_SOCK {0, PTHREAD_MUTEX_INITIALIZER}

/* 使用CVS_TRY_LOCK 编译器.h 的宏，通过给函数加注解，使用编译器自带的分析工具分析程序
 * 可能存在的并发问题。
 * 这个函数的作用是初始化一个once对象，设置done为false，mutex为一个新的互斥锁。
 * 告诉编译器，当返回值是true时，表示这个函数成功获取了once->mutex锁。
 * */
static inline bool cvsthread_once_start(struct cvsthread_once *once)
CVS_TRY_LOCK(true,once->mutex);

bool cvsthread_once_start_(struct cvsthread_once *once)
CVS_TRY_LOCK(true,once->mutex);

static inline bool cvs_thread_once_start(struct cvsthread_once *once)
{
    /* 是一个宏，提示编译器 !once->done 的情况发生的概率较低，编译器可以基于此进行优化，减少条件判断的开销。
     * 这个宏通常会影响 CPU 分支预测逻辑，帮助提高性能，特别是在大多数情况下 once->done 已经是 true，不会再执行初始化时。
     * */
    return CVS_UNLIKELY(!once->done && cvsthread_once_start_(once));
}
void ovsthread_once_done(struct cvsthread_once *once)
CVS_RELEASES(once->mutex);

#endif //CVS_THREAD_H
