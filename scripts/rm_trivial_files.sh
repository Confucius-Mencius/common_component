#!/bin/bash

###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

SCRIPT_PATH=$(cd `dirname $0`; pwd)

cd ${SCRIPT_PATH}/../build
rm cmake -rf
rm output -rf

cd ${SCRIPT_PATH}/..
find . -name "version.h" | xargs rm -rf
find . -name "*.log*" | xargs rm -rf
find . -name "*.pyc" | xargs rm -rf
find . -name ".idea" | xargs rm -rf
find . -name "cmake-build-debug" | xargs rm -rf
find . -name "cmake-build-release" | xargs rm -rf
