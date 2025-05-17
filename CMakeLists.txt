include(FetchContent)
cmake_minimum_required(VERSION 3.22)
project(cvs C)


set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-g)

# 配合本地缓存使用（例如你事先下载好依赖）
set(FETCHCONTENT_FULLY_DISCONNECTED OFF)
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(CMAKE_TLS_VERIFY ON)
# 设置下载目录
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/third-party/fetch)
set(FETCHCONTENT_SOURCE_DIR ${FETCHCONTENT_BASE_DIR}/src)
set(FETCHCONTENT_BINARY_DIR ${FETCHCONTENT_BASE_DIR}/build)

# 添加libewvent
#set(FETCHCONTENT_SOURCE_DIR_LIBEVENT ${FETCHCONTENT_BASE_DIR})
FetchContent_Declare(
        libevent
        GIT_REPOSITORY https://github.com/libevent/libevent.git
        GIT_TAG release-2.1.12-stable
)
FetchContent_MakeAvailable(libevent)

# 添加 cJSON 库
add_library(cjson third-party/cJSON/cJSON.c)
target_include_directories(cjson PUBLIC ${CMAKE_SOURCE_DIR}/third-party/cJSON)

# 递归收集所有源文件（排除 third-party/cJSON/tests）
file(GLOB_RECURSE ALL_SOURCES RELATIVE ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include/cvs/*.h
        ${CMAKE_SOURCE_DIR}/include/cvsdb/*.h
        ${CMAKE_SOURCE_DIR}/include/utils/*.h
        ${CMAKE_SOURCE_DIR}/include/netdev/*.h
        ${CMAKE_SOURCE_DIR}/include/server/*.h
        ${CMAKE_SOURCE_DIR}/lib/*.c
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
target_link_libraries(cvs cjson event)


message(STATUS "Include directories: ${CMAKE_INCLUDE_PATH}")
message(STATUS "Source files: ${ALL_SOURCES}")
