cmake_minimum_required(VERSION 3.22)
project(cvs C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)  # 强制使用 C99
add_compile_options(-g)  # 添加调试符号


include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/third-party)
include_directories(${CMAKE_SOURCE_DIR}/include)

# 定义库（必须包含 .c 源文件）
add_library(cvs_lib
        vswitchd/cvs-vswitchd.c  # 确保此文件存在
        include/cvs/cmd.c)
# 第三方库
add_library(cjson third-party/cJSON/cJSON.c)


# 定义可执行文件（假设入口是 vswitchd/cvs-vswitchd.c）
add_executable(cvs vswitchd/cvs-vswitchd.c)
#trget_include_directories(cvs_app PRIVATE ${CMAKE_SOURCE_DIR}/include/cvs)

# 链接库
target_link_libraries(cvs cvs_lib cjson)
