
#ifndef CVS_COMPILER_H
#define CVS_COMPILER_H

/*
 * 一些编译器相关的宏定义
 * 作用是可以针对不同的编译器，如gcc clang 添加不同的属性，通过注解告诉编译器，让编译器自动分析潜在的并发访问问题。
 *
 * 当函数的返回值等于 RETVAL 时，表示这个“尝试获取锁”的函数成功获取了锁。
 *
 * 这里通过定义宏来区分不同的编译器，确保在 Clang 和 GCC 下使用合适的属性：
 * - Clang 支持 shared_trylock_function 和 unlock_function 属性
 * - GCC 不支持 shared_trylock_function 属性，因此在 GCC 下不使用这些属性
 *
 */

/* Clang 编译器的 */
#if defined(__clang__)
    // 对于 Clang，支持 shared_trylock_function 属性
    /* 当函数的返回值等于 RETVAL 时，表示这个“尝试获取锁”的函数成功获取了锁。 */
    #define CVS_TRY_LOCK(RETVAL, ...) \
        __attribute__((shared_trylock_function(RETVAL, __VA_ARGS__)))

    #define CVS_RELEASES(...) \
        __attribute__((unlock_function(__VA_ARGS__))) \
/* GCC 编译器的 */
#elif defined(__GNUC__)
    // 对于 GCC，GCC 不支持 shared_trylock_function 属性，因此定义为空
    #define CVS_TRY_LOCK(RETVAL, ...)
    #define CVS_RELEASES(...)
    /*
     * 用于向编译器提供关于某个条件成立与否的提示，以帮助优化性能，尤其是在分支预测方面。
     *
     * __builtin_expect：这是 GCC 提供的一个内建函数，用来给编译器提供条件判断的期望结果。它帮助编译器更好地进行分支预测，优化代码执行路径。
     * !!(CONDITION)：这一部分确保条件的结果被强制转换为布尔值（0 表示假，1 表示真）。!! 是 C 语言中常用的双重取反技巧，能将任何真值转换为 1，将假值转换为 0。
     * __builtin_expect 的第二个参数是一个提示，表示该条件不太可能为真。也就是说，OVS_UNLIKELY 的意思是条件通常不成立（即期望结果是 false）
     * */
    #define CVS_UNLIKELY(CONDITION) __builtin_expect(!!(CONDITION), 0)

#else
    #error "Unsupported compiler!"
#endif // __clang__ or __GNUC__

#endif // CVS_COMPILER_H
