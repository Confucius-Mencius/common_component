###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

cmake_minimum_required(VERSION 2.8)

###############################################################################
# project name
set(PROJ_NAME base0)
project(${PROJ_NAME} CXX)

set(CXX_LANGUAGE on CACHE INTERNAL "this is a cxx language project")

###############################################################################
# third party
set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/../third_party)
include(${THIRD_PARTY_DIR}/cmake/third_party.cmake)

###############################################################################
# 启用-Werror
SetCompileFlags(TRUE)

###############################################################################
# output dir
SetOutputDir(${PROJECT_SOURCE_DIR}/../build/output "")

###############################################################################
# 打印编译选项
ShowBuildOptions()

###############################################################################
# project inc dir
set(PROJ_INC_DIR ${PROJECT_SOURCE_DIR}/inc)

if (NOT COMPILER_SUPPORTS_CXX11)
    set(OPENCC_INC_DIR "")
    set(OPENCC_LIB_DIR "")
    set(OPENCC_LIB_NAME "")
endif ()

###############################################################################
# sub dir
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(test/valgrind)
add_subdirectory(test/libevent)
add_subdirectory(test/hostname_ip_conv)

if (COMPILER_SUPPORTS_CXX11)
    add_subdirectory(test/flatbuf)
endif ()

add_subdirectory(test/multi_thread)
add_subdirectory(test/libevhtp)
# add_subdirectory(test/udt)
# add_subdirectory(test/json2pb)
add_subdirectory(test/libwebsockets)
add_subdirectory(test/libwebsockets/http)
add_subdirectory(test/libwebsockets/ws)
add_subdirectory(test/libwebsockets/ws_libevent)
add_subdirectory(test/log4cplus)
add_subdirectory(test/tcp_client_bind)
