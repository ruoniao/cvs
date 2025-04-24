//
// Created by zhaogang on 2025-04-23.
//
#include "cvs/thread.h"


/*ovsthread_once_start 直接实现 */
//TODO: 这里的实现可以优化，因为这里的需要是用线程锁保证只执行一次，所以大部分都是获取不到锁的，可以使用unlike优化
static bool cvsthread_once_start_(struct cvsthread_once *once)
{

    pthread_mutex_lock(&once->mutex);
    if (!once->done) {
        return true;
    }
    pthread_mutex_unlock(&once->mutex);
    return false;
}


void ovsthread_once_done(struct cvsthread_once *once)
{

    /*
      这行代码在执行时会插入一个“释放型”内存屏障，确保当前线程在执行 once->done = true; 操作之前，所有先前的内存操作（如初始化操作）不会被重新排序到
     once->done = true; 之后执行。具体来说，memory_order_release 保证在此内存屏障之前的所有操作在该语句执行之前都对其他线程可见。

    内存屏障通过在代码中插入特殊的指令或操作，确保某些内存操作的执行顺序。它们可以防止编译器或处理器在优化时重排内存访问指令，从而确保数据一致性。
     * */
    atomic_thread_fence(memory_order_release);

    once->done = true;
    /*
     * 这里的mutex是一个互斥锁，使用pthread_mutex_unlock来释放锁
     * */
    /*
     * 这里只释放锁，常规看来可能会认为是错误的，因为没有加锁就释放锁，其实在cvsthread_once_start_函数中已经加锁了return true;并没有释放
     * */
    pthread_mutex_unlock(&once->mutex);
}