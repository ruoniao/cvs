cmake_minimum_required(VERSION 3.22)
project(cvs C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-g)

# 添加第三方库 cJSON
add_library(cjson third-party/cJSON/cJSON.c)
target_include_directories(cjson PUBLIC ${CMAKE_SOURCE_DIR}/third-party/cJSON)

# 递归收集所有 .c 文件（排除 third-party/cJSON/tests 目录）
file(GLOB_RECURSE ALL_SOURCES RELATIVE ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/*.c
        ${CMAKE_SOURCE_DIR}/include/cvs/*.c
        ${CMAKE_SOURCE_DIR}/lib/*.c
        )

# 过滤掉包含 "third-party/cJSON/tests/" 的文件
list(FILTER ALL_SOURCES EXCLUDE REGEX "third-party/cJSON/tests/.*")

# 定义库
add_library(cvs_lib ${ALL_SOURCES})
target_include_directories(cvs_lib PUBLIC
        ${CMAKE_SOURCE_DIR}/lib
        ${CMAKE_SOURCE_DIR}/third-party/cJSON
        ${CMAKE_SOURCE_DIR}/include
        )

# 定义可执行文件
add_executable(cvs vswitchd/cvs-vswitchd.c)
target_include_directories(cvs PUBLIC
        ${CMAKE_SOURCE_DIR}/lib
        ${CMAKE_SOURCE_DIR}/third-party/cJSON
        ${CMAKE_SOURCE_DIR}/include
        )

# 链接库
target_link_libraries(cvs cvs_lib cjson)
