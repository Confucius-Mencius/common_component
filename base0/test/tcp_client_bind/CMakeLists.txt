###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

###############################################################################
# output dir
set(TCP_CLIENT_BIND_TEST_OUTPUT_DIR ${OUTPUT_DIR}/test/tcp_client_bind)
MakeDir(${TCP_CLIENT_BIND_TEST_OUTPUT_DIR})

###############################################################################
# target and linked library
add_executable(client client.cpp)
add_executable(server server.cpp)

set_target_properties(client PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TCP_CLIENT_BIND_TEST_OUTPUT_DIR})
set_target_properties(server PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TCP_CLIENT_BIND_TEST_OUTPUT_DIR})
