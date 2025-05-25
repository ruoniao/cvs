//
// Created by zhaogang on 2025-05-17.
//

#ifndef CVS_CHECK_H
#define CVS_CHECK_H

#if __GNUC__ && !defined(__cplusplus)
/* return 0 for array types, 1 otherwise */
#define __ARRAY_SIZE_NOCHECK(ARRAY) (sizeof(ARRAY) / sizeof((ARRAY)[0]))

#define __ARRAY_CHECK(ARRAY) 					\
    !__builtin_types_compatible_p(typeof(ARRAY), typeof(&ARRAY[0]))

/* compile-time fail if not array */
#define __ARRAY_FAIL(ARRAY) (sizeof(char[-2*!__ARRAY_CHECK(ARRAY)]))
#define __ARRAY_SIZE(ARRAY)					\
    __builtin_choose_expr(__ARRAY_CHECK(ARRAY),			\
        __ARRAY_SIZE_NOCHECK(ARRAY), __ARRAY_FAIL(ARRAY))

#endif

/* Returns the number of elements in ARRAY. */
#define ARRAY_SIZE(ARRAY) __ARRAY_SIZE(ARRAY)

int health_check_handler();


#endif //CVS_CHECK_H
