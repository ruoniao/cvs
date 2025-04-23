//
// Created by zhaogang on 2025-04-23.
//

#ifndef CVS_THREAD_H
#define CVS_THREAD_H
#include <stdatomic.h>
#include <stdbool.h>
#include "pthread.h"
#include "compiler.h"

struct ovsthread_once {
    int done;
    pthread_mutex_t mutex;
};

/* 使用CVS_TRY_LOCK 编译器.h 的宏，通过给函数加注解，使用编译器自带的分析工具分析程序
 * 可能存在的并发问题。
 * 这个函数的作用是初始化一个once对象，设置done为false，mutex为一个新的互斥锁。
 * 告诉编译器，当返回值是true时，表示这个函数成功获取了once->mutex锁。
 * */
static inline bool ovsthread_once_start(struct ovsthread_once *once)
CVS_TRY_LOCK(true,once->mutex);


/*ovsthread_once_start 直接实现 */
//TODO: 这里的实现可以优化，因为这里的需要是用线程锁保证只执行一次，所以大部分都是获取不到锁的，可以使用unlike优化
static inline bool ovsthread_once_start(struct ovsthread_once *once)
{

    pthread_mutex_lock(&once->mutex);
    if (!once->done) {
        return true;
    }
    pthread_mutex_unlock(&once->mutex);
    return false;
}
#endif //CVS_THREAD_H
