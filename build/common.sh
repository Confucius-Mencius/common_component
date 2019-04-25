#!/bin/bash

###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

CODE_BASE_DIR=${SCRIPT_PATH}/..
COMMON_COMPONENT_INSTALL_PREFIX=/opt/common_component # common component的安装目录，可以根据实际情况修改

PROJ_LIST=(base log_engine conf_center timer_axis thread_center proto_msg_codec trans_center 
	global_thread work_threads burden_threads tcp_threads proto_tcp_threads http_ws_threads
	app_frame app_launcher)
