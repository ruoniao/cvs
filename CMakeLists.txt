cmake_minimum_required(VERSION 3.22)
project(cvs C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-g)


# 添加 cJSON 库
add_library(cjson third-party/cJSON/cJSON.c)
target_include_directories(cjson PUBLIC ${CMAKE_SOURCE_DIR}/third-party/cJSON)

# 递归收集所有源文件（排除 third-party/cJSON/tests）
file(GLOB_RECURSE ALL_SOURCES RELATIVE ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include/cvs/*.c
        ${CMAKE_SOURCE_DIR}/include/cvsdb/*.c
        ${CMAKE_SOURCE_DIR}/include/utils/*.c
        )

list(FILTER ALL_SOURCES EXCLUDE REGEX "third-party/cJSON/tests/.*")

# 定义可执行文件，直接包含所有源文件
add_executable(cvs vswitchd/cvs-vswitchd.c ${ALL_SOURCES})

# 设置头文件搜索路径（包含目录)
set(CMAKE_INCLUDE_PATH
        ${CMAKE_SOURCE_DIR}/lib
        ${CMAKE_SOURCE_DIR}/third-party/cJSON
        ${CMAKE_SOURCE_DIR}/include
        )
target_include_directories(cvs PUBLIC ${CMAKE_INCLUDE_PATH})

# 设置在链接阶段需要链接的库，这里是 cJSON 库
target_link_libraries(cvs cjson)


message(STATUS "Include directories: ${CMAKE_INCLUDE_PATH}")
message(STATUS "Source files: ${ALL_SOURCES}")
