###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

set(SERVER libevent_multi_thread_io_test_server)
set(CLIENT libevent_multi_thread_io_test_client)
set(CLIENT_2 libevent_multi_thread_io_test_client_2)
set(CLIENT_3 libevent_multi_thread_io_test_client_3)

###############################################################################
# include/link dir
include_directories(${LIBEVENT_INC_DIR} ${LOG4CPLUS_INC_DIR})
link_directories(${LIBEVENT_LIB_DIR} ${LOG4CPLUS_LIB_DIR} ${OPENSSL_LIB_DIR} ${GPERFTOOLS_LIB_DIR})

###############################################################################
# target
add_executable(${SERVER} server.cpp)
add_executable(${CLIENT} client.cpp)
add_executable(${CLIENT_2} client2.cpp)
add_executable(${CLIENT_3} client3.cpp)

###############################################################################
# library depend
target_link_libraries(${SERVER} ${LIBEVENT_LIB_NAME} ${LOG4CPLUS_LIB_NAME} ${GPERFTOOLS_LIB_NAME} pthread)
target_link_libraries(${CLIENT} ${LIBEVENT_LIB_NAME} ${LOG4CPLUS_LIB_NAME} ${GPERFTOOLS_LIB_NAME})
target_link_libraries(${CLIENT_2} ${LIBEVENT_LIB_NAME} ${LOG4CPLUS_LIB_NAME} ${GPERFTOOLS_LIB_NAME})
target_link_libraries(${CLIENT_3} ${LIBEVENT_LIB_NAME} ${LOG4CPLUS_LIB_NAME} ${GPERFTOOLS_LIB_NAME})

set_target_properties(${SERVER} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${LIBEVENT_TEST_OUTPUT_DIR})
set_target_properties(${CLIENT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${LIBEVENT_TEST_OUTPUT_DIR})
set_target_properties(${CLIENT_2} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${LIBEVENT_TEST_OUTPUT_DIR})
set_target_properties(${CLIENT_3} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${LIBEVENT_TEST_OUTPUT_DIR})
