###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

set(HOSTNAME_TO_IP hostname_to_ip)
set(IP_TO_HOSTNAME ip_to_hostname)

###############################################################################
# target
add_executable(${HOSTNAME_TO_IP} hostname_to_ip.cpp)
add_executable(${IP_TO_HOSTNAME} ip_to_hostname.cpp)

set(HOSTNAME_IP_CONV_TEST_OUTPUT_DIR ${OUTPUT_DIR}/test/hostname_ip_conv)
MakeDir(${HOSTNAME_IP_CONV_TEST_OUTPUT_DIR})

set_target_properties(${HOSTNAME_TO_IP} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${HOSTNAME_IP_CONV_TEST_OUTPUT_DIR})
set_target_properties(${IP_TO_HOSTNAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${HOSTNAME_IP_CONV_TEST_OUTPUT_DIR})
