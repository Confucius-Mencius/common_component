###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

cmake_minimum_required(VERSION 2.8)

###############################################################################
# project name
set(PROJ_NAME base)
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
