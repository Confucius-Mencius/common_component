#!/bin/bash

###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

SCRIPT_PATH=$(cd `dirname $0`; pwd)

OLD_VERSION=1.0.0
NEW_VERSION=1.0.1
DIR=${SCRIPT_PATH}/..

sed -i "s/set(BUILD_VERSION ${OLD_VERSION})/set(BUILD_VERSION ${NEW_VERSION})/g" `grep "set(BUILD_VERSION ${OLD_VERSION})" -rl --exclude=${SCRIPT_PATH}/*.sh ${DIR}`
