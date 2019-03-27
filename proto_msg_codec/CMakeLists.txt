###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

cmake_minimum_required(VERSION 2.8)

###############################################################################
# project name
set(PROJ_NAME proto_msg_codec)
project(${PROJ_NAME} NONE)

###############################################################################
# third party
set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/../third_party)
include(${THIRD_PARTY_DIR}/cmake/third_party.cmake)

###############################################################################
# project inc dir
set(PROJ_INC_DIR ${PROJECT_SOURCE_DIR}/inc)

###############################################################################
# install lib and include files
InstallHeaderFiles(${PROJ_INC_DIR} include)
