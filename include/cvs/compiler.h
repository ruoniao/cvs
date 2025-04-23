//
// Created by zhaogang on 2025-04-23.
//
/*
 * 一些编译器相关的宏定义
 * 作用是可以针对不同的编译器，如gcc clang 添加不同的属性，通过注解告诉编译器，让编译器自动分析潜在的并发访问问题。
 *
 *
 *
 * */
#ifndef CVS_COMPILER_H
#define CVS_COMPILER_H

/* 当函数的返回值等于 RETVAL 时，表示这个“尝试获取锁”的函数成功获取了锁。 */
#define CVS_TRY_LOCK(RETVAL, ...) \
    __attribute__((shared_trylock_function(RETVAL, __VA_ARGS__)))
#endif //CVS_COMPILER_H
