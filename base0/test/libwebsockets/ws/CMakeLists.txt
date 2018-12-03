###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

###############################################################################
# output dir
set(LIBWEBSOCKETS_TEST_OUTPUT_DIR ${OUTPUT_DIR}/test/websockets/ws)
MakeDir(${LIBWEBSOCKETS_TEST_OUTPUT_DIR})

###############################################################################
# include/link dir
include_directories(${PROJ_INC_DIR} ${LIBWEBSOCKETS_INC_DIR} ${LIBEVENT_INC_DIR} ${OPENSSL_INC_DIR})
link_directories(${LIBWEBSOCKETS_LIB_DIR} ${LIBEVENT_LIB_DIR} ${OPENSSL_LIB_DIR} ${GPERFTOOLS_LIB_DIR})

## ws server
set(SERVER_NAME minimal-ws-server)

###############################################################################
# target and linked library
add_executable(${SERVER_NAME} minimal-ws-server.cpp)

target_link_libraries(${SERVER_NAME} ${LIBWEBSOCKETS_LIB_NAME} ${LIBEVENT_LIB_NAME} ${OPENSSL_LIB_NAME} ${GPERFTOOLS_LIB_NAME})

set_target_properties(${SERVER_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${LIBWEBSOCKETS_TEST_OUTPUT_DIR})

## ws client
set(CLIENT_NAME minimal-ws-client)

###############################################################################
# target and linked library
add_executable(${CLIENT_NAME} minimal-ws-client.cpp)

target_link_libraries(${CLIENT_NAME} ${LIBWEBSOCKETS_LIB_NAME} ${LIBEVENT_LIB_NAME} ${OPENSSL_LIB_NAME} ${GPERFTOOLS_LIB_NAME})

set_target_properties(${CLIENT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${LIBWEBSOCKETS_TEST_OUTPUT_DIR})
