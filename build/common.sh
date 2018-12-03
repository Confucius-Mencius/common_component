#!/bin/bash

###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

CODE_BASE_DIR=${SCRIPT_PATH}/..
COMMON_COMPONENT_INSTALL_PREFIX=/opt/common_component # common component的安装目录，根据实际情况修改

# PROJ_LIST=(base log_engine conf_center timer_axis msg_codec_center trans_center random_engine time_service
#     thread_center conn_center_mgr client_center_mgr global_thread work_threads burden_threads tcp_threads 
#     http_threads udp_threads app_frame app_launcher event_center script_engine words_filter)

PROJ_LIST=(base log_engine conf_center timer_axis msg_codec_center trans_center random_engine time_service
    thread_center conn_center_mgr client_center_mgr global_thread work_threads burden_threads tcp_threads 
    http_threads udp_threads app_frame app_launcher script_engine)
