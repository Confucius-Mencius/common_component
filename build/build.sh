#!/bin/bash

###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

SCRIPT_PATH=$(cd `dirname $0`; pwd)

function Usage()
{
    echo "Usage: $1
         -h                show this help info.
         -b debug|release  build projects.
         -r debug|release  rebuild projects.
         -c debug|release  clear tmp files."
    exit 0   
}

if [ $# == 0 ]; then
    Usage $0
fi

. ${SCRIPT_PATH}/../../sh_tools/base/util.sh
. ${SCRIPT_PATH}/../../sh_tools/base/proj_util.sh
. ${SCRIPT_PATH}/common.sh

# 编译的时候是否执行单元测试用例
RUN_TEST=0

while getopts "b:c:r:h" opt; do
    case $opt in
        h)
            Usage $0
        ;;
        b)
            if [ "${OPTARG}"x = "debug"x ]; then
                COMMON_COMPONENT_INSTALL_PREFIX=${COMMON_COMPONENT_INSTALL_PREFIX}/debug
                OUTPUT_DIR=${SCRIPT_PATH}/output/debug
                BUILD_PARENT_DIR=${SCRIPT_PATH}/cmake/debug
                BUILD_TYPE=Debug
            elif [ "${OPTARG}"x = "release"x ]; then
                COMMON_COMPONENT_INSTALL_PREFIX=${COMMON_COMPONENT_INSTALL_PREFIX}/release
                OUTPUT_DIR=${SCRIPT_PATH}/output/release
                BUILD_PARENT_DIR=${SCRIPT_PATH}/cmake/release
                BUILD_TYPE=Release
            else
                echo "invalid build type: ${OPTARG}"
                exit 1
            fi
            
            MakeDir ${OUTPUT_DIR}

            ###############################################################################
            for i in ${PROJ_LIST[@]}; do
                PROJ=$i

                cd ${CODE_BASE_DIR}
                BuildProj ${PROJ} ${SCRIPT_PATH}/../${PROJ} ${BUILD_PARENT_DIR}/${PROJ} ${BUILD_TYPE} ${COMMON_COMPONENT_INSTALL_PREFIX} ${RUN_TEST}

                cd ${CODE_BASE_DIR}
                InstallProj ${PROJ} ${BUILD_PARENT_DIR}/${PROJ}
            done
        ;;
        r)
            if [ "${OPTARG}"x = "debug"x ]; then
                COMMON_COMPONENT_INSTALL_PREFIX=${COMMON_COMPONENT_INSTALL_PREFIX}/debug
                OUTPUT_DIR=${SCRIPT_PATH}/output/debug
                BUILD_PARENT_DIR=${SCRIPT_PATH}/cmake/debug
                BUILD_TYPE=Debug
            elif [ "${OPTARG}"x = "release"x ]; then
                COMMON_COMPONENT_INSTALL_PREFIX=${COMMON_COMPONENT_INSTALL_PREFIX}/release
                OUTPUT_DIR=${SCRIPT_PATH}/output/release
                BUILD_PARENT_DIR=${SCRIPT_PATH}/cmake/release
                BUILD_TYPE=Release
            else
                echo "invalid rebuild type: ${OPTARG}"
                exit 1
            fi
            
            MakeDir ${OUTPUT_DIR}

            ###############################################################################
            for i in ${PROJ_LIST[@]}; do
                PROJ=$i

                cd ${CODE_BASE_DIR}
                RebuildProj ${PROJ} ${SCRIPT_PATH}/../${PROJ} ${BUILD_PARENT_DIR}/${PROJ} ${BUILD_TYPE} ${COMMON_COMPONENT_INSTALL_PREFIX} ${RUN_TEST}

                cd ${CODE_BASE_DIR}
                InstallProj ${PROJ} ${BUILD_PARENT_DIR}/${PROJ}
            done
        ;;
        c)
            if [ "${OPTARG}"x = "debug"x ]; then
                BUILD_PARENT_DIR=${SCRIPT_PATH}/cmake/debug
            elif [ "${OPTARG}"x = "release"x ]; then
                BUILD_PARENT_DIR=${SCRIPT_PATH}/cmake/release
            else
                echo "invalid clear type: ${OPTARG}"
                exit 1
            fi

            ###############################################################################
            for i in ${PROJ_LIST[@]}; do
                PROJ=$i
                cd ${CODE_BASE_DIR}
                ClearProj ${PROJ} ${BUILD_PARENT_DIR}/${PROJ}
            done
        ;;
        \?)
            echo "invalid option"
        ;;
    esac
done
