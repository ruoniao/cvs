################################# 1. 设置一些基本的 CMake 配置 #################################

# 引入 FetchContent 模块 的语句，其作用是为项目启用一种现代方式来自动下载、构建和集成外部依赖库，
# 替代传统的手动下载或使用 add_subdirectory() 的方式。
include(FetchContent)
cmake_minimum_required(VERSION 3.22.1)
# 新的cmake已经不允许使用旧的策略 CMP0002强制允许也不可以了。所以还是会存在在fetch多个cmake项目时，若每个项目都有uninstall,会导致冲突的问题，
# 因为cmake 只允许一个名称为 uninstall 的目标（add_executable构建的）
cmake_policy(SET CMP0002 OLD)

project(cvs
        VERSION 0.1.0
        LANGUAGES C)

# 设置include变量,供后面FetchContent添加
set(CMAKE_INCLUDE_PATH
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-g)

################################# 2. 设置FetchContent的下载目录等配置 ##############################
# 配合本地缓存使用（例如你事先下载好依赖）
set(FETCHCONTENT_FULLY_DISCONNECTED OFF)  #OFF 允许 FetchContent 访问网络来下载依赖项,ON 表示完全断网模式
set(FETCHCONTENT_QUIET ON) # ON 开启详细输出
set(FETCHCONTENT_UPDATES_DISCONNECTED OFF) # OFF允许联网从远程拉取依赖,ON 表示只从本地缓存拉取，不会尝试联网下载
set(CMAKE_TLS_VERIFY ON)
# 设置下载目录
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/third-party/fetch)
set(FETCHCONTENT_SOURCE_DIR ${FETCHCONTENT_BASE_DIR}/src)
set(FETCHCONTENT_BINARY_DIR ${FETCHCONTENT_BASE_DIR}/build)

################################# 3. 通过FetchContent下载libewvent库##############################
FetchContent_Declare(
        libevent
        GIT_REPOSITORY https://github.com/libevent/libevent.git
        GIT_TAG release-2.1.12-stable
)

# FetchContent_MakeAvailable() 使用，当target_link_libraries时设置了event,自动下载并添加目标，无需手动设置 include_directories
FetchContent_MakeAvailable(libevent)

################################# 3. cmake传统方式设置cJson库##############################
# 传统的方式使用cjson,不再使用
# add_library(cjson third-party/cJSON/cJSON.c)
# target_include_directories(cjson PUBLIC ${CMAKE_SOURCE_DIR}/third-party/cJSON)
# （排除 third-party/cJSON/tests）
#list(FILTER ALL_SOURCES EXCLUDE REGEX "third-party/cJSON/tests/.*")


################################# 4. 通过FetchContent下载cJson库##############################
# 添加 cJSON 库
# 关闭cjson的fuzzing功能，避免编译时出现错误。对于remote开发，可能需要从远端拉取到本地避免clion cmake api 报错
set(ENABLE_FUZZING OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
        cjson
        GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
        GIT_TAG v1.7.18
)
FetchContent_MakeAvailable(cjson)

################################## 5. cmake传统方式设置dpdk 库##############################
# === DPDK 设置 ===
#set(DPDK_DIR /usr/local)
#set(DPDK_INCLUDE_PATH
#        ${DPDK_DIR}/lib/dpdk
#        ${DPDK_DIR}/include/dpdk
#        )
#set(DPDK_LIBS
#        rte_eal
#        rte_mempool
#        rte_mbuf
#        rte_ring
#        rte_ethdev
#        rte_kvargs
#        )

################################## 6. PkgConfig方式设置dpdk库 ##############################
# 设置libdpdk.pc的路径。这里我在安装dpdk时指定到了/usr/local/lib/dpdk/pkgconfig
# 这里设置DPDK_DIR 没起作用，需要手动copy /usr/local/lib/dpdk/pkgconfig/libdpdk* /usr/lib/pkgconfig
# PkgConfig方式 不用记住各种库的头文件路径、库文件路径、以及它们的依赖关系，它帮你自动把这些参数输出给编译器和链接器，尤其是复杂项目或者多依赖库时简单。
# 警示：⚠️install 时尽量不设置prefix路径，否则可能会导致编译时找不到库文件。
set(DPDK_DIR "/usr/local/lib/dpdk/pkgconfig")
find_package(PkgConfig REQUIRED)
pkg_check_modules(DPDK REQUIRED libdpdk)

# PkgConfig 需要手动 include 和 link。
# PkgConfig 只是在config和编译记录了 include lib 的路径而已
set(CMAKE_INCLUDE_PATH
        ${CMAKE_INCLUDE_PATH}
        ${DPDK_INCLUDE_DIRS}
        )
# 将PkgConfig 分析出的 DPDK_INCLUDE_DIRS 添加到 CMake 的 link_directories 路径中
link_directories(${DPDK_LIBRARY_DIRS})
#set(DPDK_LIBS
#        rte_eal
#        rte_mempool
#        rte_mbuf
#        rte_ring
#        rte_ethdev
#        rte_kvargs
#        )

################################## 7. 构建本项目CVS编译条件 ##############################


################################## 7.1 收集所有文件 #####################################
# 递归收集所有源文件（排除 third-party/cJSON/tests）定义在ALL_SOURCES变量
# file 是 CMake 的一个命令，用于操作文件和目录，比如查找文件、创建目录等。
# GLOB_RECURSE 是 file 命令的一个子命令，表示递归地查找匹配的文件。它会从指定目录开始，递归进入所有子目录，收集匹配指定模式的文件路径。
# ALL_SOURCES 是一个变量名，用来存储 GLOB_RECURSE 找到的所有文件路径列表。
# RELATIVE ${CMAKE_SOURCE_DIR} 表示所有返回的文件路径将相对于 ${CMAKE_SOURCE_DIR} 这个路径进行存储，而不是绝对路径。
# ${CMAKE_SOURCE_DIR} 是 CMake 内置变量，代表当前工程的根目录路径。
file(GLOB_RECURSE ALL_SOURCES
    RELATIVE ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include/*.h
    ${CMAKE_SOURCE_DIR}/lib/*.c
    #${CMAKE_SOURCE_DIR}/vswitchd/*.c
)
message(STATUS "Source files: ${ALL_SOURCES}")

################################## 7.2  定义可执行文件cvs #####################################
# cvs-vswitchd.c 是主程序文件，包含了程序的入口点和主要逻辑。
add_executable(cvs vswitchd/cvs-vswitchd.c ${ALL_SOURCES})

################################## 7.3  定义include头文件搜索路径 #####################################
# 设置头文件搜索路径（包含目录)
set(CMAKE_INCLUDE_PATH
        ${CMAKE_INCLUDE_PATH}
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/lib
        # 第三方库可能没有规范，保险起见这里我们手动添加
        ${cjson_SOURCE_DIR}     # cjson的源代码目录

        )
message(STATUS "Include directories: ${CMAKE_INCLUDE_PATH}")
target_include_directories(cvs PRIVATE ${CMAKE_INCLUDE_PATH})


################################## 7.4  定义target 链接路径 #####################################
# 设置链接库路径;  这里引入的第三方库有 libevent 和 cJSON
target_link_libraries(cvs PRIVATE cjson event ${DPDK_LIBRARIES})




# === 单元测试二进制 ===
add_executable(test_cvsdb tests/test_cvsdb.c ${ALL_SOURCES})
target_include_directories(test_cvsdb PUBLIC ${CMAKE_INCLUDE_PATH} ${DPDK_INCLUDE_PATH})
target_link_libraries(test_cvsdb cjson event ${DPDK_LIBRARIES})

