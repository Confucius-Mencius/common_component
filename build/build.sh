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
         -c debug|release  clear tmp files.
         [PROJ_LIST]" # eg. "log_engine timer_axis"

    exit 0   
}

if [ $# == 0 ]; then
    Usage $0
fi

. ${SCRIPT_PATH}/../../sh_tools/base/util.sh
. ${SCRIPT_PATH}/../../sh_tools/base/proj_util.sh
. ${SCRIPT_PATH}/common.sh

# 编译的时候是否执行单元测试用例
RUN_TEST=1

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

            # 可以由外部输入需要编译的proj list，替换预定义的值。格式：./build.sh -b debug "log_engine timer_axis"，需要几个就写几个，用空格分隔
            if [ $# == 3 ]; then
                PROJ_LIST=($3)
            fi

            ###############################################################################
            for i in ${PROJ_LIST[@]}; do
                PROJ=$i
                echo "----------" ${PROJ} "----------"

                cd ${CODE_BASE_DIR}
                BuildProj ${PROJ} ${SCRIPT_PATH}/../${PROJ} ${BUILD_PARENT_DIR}/${PROJ} ${BUILD_TYPE} ${COMMON_COMPONENT_INSTALL_PREFIX} ${RUN_TEST}

                cd ${CODE_BASE_DIR}
                InstallProj ${PROJ} ${BUILD_PARENT_DIR}/${PROJ}

                echo ""
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

            # 可以由外部输入需要重新编译的proj list，替换预定义的值。格式：./build.sh -r debug "log_engine timer_axis"，需要几个就写几个，用空格分隔
            if [ $# == 3 ]; then
                PROJ_LIST=($3)
            fi

            ###############################################################################
            for i in ${PROJ_LIST[@]}; do
                PROJ=$i
                echo "----------" ${PROJ} "----------"

                cd ${CODE_BASE_DIR}
                RebuildProj ${PROJ} ${SCRIPT_PATH}/../${PROJ} ${BUILD_PARENT_DIR}/${PROJ} ${BUILD_TYPE} ${COMMON_COMPONENT_INSTALL_PREFIX} ${RUN_TEST}

                cd ${CODE_BASE_DIR}
                InstallProj ${PROJ} ${BUILD_PARENT_DIR}/${PROJ}

                echo ""
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

            # 可以由外部输入需要清理的proj list，替换预定义的值。格式：./build.sh -c debug "log_engine timer_axis"，需要几个就写几个，用空格分隔
            if [ $# == 3 ]; then
                PROJ_LIST=($3)
            fi

            ###############################################################################
            for i in ${PROJ_LIST[@]}; do
                PROJ=$i
                echo "----------" ${PROJ} "----------"

                cd ${CODE_BASE_DIR}
                ClearProj ${PROJ} ${BUILD_PARENT_DIR}/${PROJ}

                echo "done."
            done
        ;;
        \?)
            echo "invalid option"
        ;;
    esac
done
