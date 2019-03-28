###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

cmake_minimum_required(VERSION 2.8)

###############################################################################
# project name
set(PROJ_NAME burden_threads)
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
# external dependence
set(BASE_INC_DIR ${PROJECT_SOURCE_DIR}/../base/inc)
set(BASE_LIB_DIR ${OUTPUT_DIR})
set(BASE_LIB_NAME base)

set(LOG_ENGINE_INC_DIR ${PROJECT_SOURCE_DIR}/../log_engine/inc)
set(CONF_CENTER_INC_DIR ${PROJECT_SOURCE_DIR}/../conf_center/inc)
set(TIMER_AXIS_INC_DIR ${PROJECT_SOURCE_DIR}/../timer_axis/inc)
set(THREAD_CENTER_INC_DIR ${PROJECT_SOURCE_DIR}/../thread_center/inc)
set(APP_FRAME_INC_DIR ${PROJECT_SOURCE_DIR}/../app_frame/inc)
set(PROTO_MSG_CODEC_INC_DIR ${PROJECT_SOURCE_DIR}/../proto_msg_codec/inc)

###############################################################################
# project inc dir
set(PROJ_INC_DIR ${PROJECT_SOURCE_DIR}/inc)

###############################################################################
# sub dir
add_subdirectory(src)
