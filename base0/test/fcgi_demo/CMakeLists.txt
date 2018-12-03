###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

cmake_minimum_required(VERSION 2.8)

###############################################################################
# project name
set(PROJ_NAME fcgi_demo)
project(${PROJ_NAME} CXX)

set(CXX_LANGUAGE on CACHE INTERNAL "this is a cxx language project")

###############################################################################
# third party
set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/../third_party)
include(${THIRD_PARTY_DIR}/cmake/third_party.cmake)

###############################################################################
# output dir
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../debug/${PROJ_NAME})
elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../release/${PROJ_NAME})
endif ()

MakeDir(${OUTPUT_DIR})

###############################################################################
# log dir
set(LOG_DIR ${OUTPUT_DIR}/log)
MakeDir(${LOG_DIR})

###############################################################################
# base lib
set(BASE_LIB_INC_DIR ${PROJECT_SOURCE_DIR}/../base/inc)
set(BASE_LIB_LIB_DIR ${OUTPUT_DIR}/..)
set(BASE_LIB_LIB_NAME base)

###############################################################################
# project inc dir
set(PROJ_INC_DIR ${PROJECT_SOURCE_DIR}/inc)

###############################################################################
# sub dir
add_subdirectory(src)
