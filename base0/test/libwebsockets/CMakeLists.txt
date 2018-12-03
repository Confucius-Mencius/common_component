###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

###############################################################################
# output dir
set(LIBWEBSOCKETS_TEST_OUTPUT_DIR ${OUTPUT_DIR}/test/websockets)
MakeDir(${LIBWEBSOCKETS_TEST_OUTPUT_DIR})

###############################################################################
# include/link dir
include_directories(${PROJ_INC_DIR} ${LIBWEBSOCKETS_INC_DIR} ${LIBEVENT_INC_DIR} ${OPENSSL_INC_DIR})
link_directories(${LIBWEBSOCKETS_LIB_DIR} ${LIBEVENT_LIB_DIR} ${OPENSSL_LIB_DIR} ${GPERFTOOLS_LIB_DIR})

##
set(CLIENT_NAME websockets_client)

###############################################################################
# target and linked library
add_executable(${CLIENT_NAME} client.cpp)

target_link_libraries(${CLIENT_NAME} ${LIBWEBSOCKETS_LIB_NAME} ${LIBEVENT_LIB_NAME} ${OPENSSL_LIB_NAME} ${GPERFTOOLS_LIB_NAME})

set_target_properties(${CLIENT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${LIBWEBSOCKETS_TEST_OUTPUT_DIR})

## libevent websockets server
set(SERVER_NAME websockets_server)

###############################################################################
# target and linked library
add_executable(${SERVER_NAME} server.cpp)

target_link_libraries(${SERVER_NAME} ${LIBWEBSOCKETS_LIB_NAME} ${LIBEVENT_LIB_NAME} ${OPENSSL_LIB_NAME} ${GPERFTOOLS_LIB_NAME})

set_target_properties(${SERVER_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${LIBWEBSOCKETS_TEST_OUTPUT_DIR})

##
set(CLIENT2_NAME websockets_client2)

###############################################################################
# target and linked library
add_executable(${CLIENT2_NAME} client2.cpp)

target_link_libraries(${CLIENT2_NAME} ${LIBWEBSOCKETS_LIB_NAME} ${LIBEVENT_LIB_NAME} ${OPENSSL_LIB_NAME} pthread ${GPERFTOOLS_LIB_NAME})

set_target_properties(${CLIENT2_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${LIBWEBSOCKETS_TEST_OUTPUT_DIR})

##
set(SERVER2_NAME websockets_server2)

###############################################################################
# target and linked library
add_executable(${SERVER2_NAME} server2.cpp)

target_link_libraries(${SERVER2_NAME} ${LIBWEBSOCKETS_LIB_NAME} ${LIBEVENT_LIB_NAME} ${OPENSSL_LIB_NAME} ${GPERFTOOLS_LIB_NAME})

set_target_properties(${SERVER2_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${LIBWEBSOCKETS_TEST_OUTPUT_DIR})

##
set(HTTP_CLIENT_NAME websockets_http_client)

###############################################################################
# target and linked library
add_executable(${HTTP_CLIENT_NAME} http_client.cpp)

target_link_libraries(${HTTP_CLIENT_NAME} ${LIBWEBSOCKETS_LIB_NAME} ${LIBEVENT_LIB_NAME} ${OPENSSL_LIB_NAME} pthread ${GPERFTOOLS_LIB_NAME})

set_target_properties(${HTTP_CLIENT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${LIBWEBSOCKETS_TEST_OUTPUT_DIR})

##
set(HTTP_CLIENT_LIBEVENT_NAME websockets_http_client_libevent)

###############################################################################
# target and linked library
add_executable(${HTTP_CLIENT_LIBEVENT_NAME} http_client_libevent.cpp)

target_link_libraries(${HTTP_CLIENT_LIBEVENT_NAME} ${LIBWEBSOCKETS_LIB_NAME} ${LIBEVENT_LIB_NAME} ${OPENSSL_LIB_NAME} pthread ${GPERFTOOLS_LIB_NAME})

set_target_properties(${HTTP_CLIENT_LIBEVENT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${LIBWEBSOCKETS_TEST_OUTPUT_DIR})
