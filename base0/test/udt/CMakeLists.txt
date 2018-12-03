###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

set(SERVER udt_server)
set(CLIENT udt_client)

###############################################################################
# include/link dir
include_directories(${LIBUDT_INC_DIR} ${LOG4CPLUS_INC_DIR})
link_directories(${LIBUDT_LIB_DIR} ${LOG4CPLUS_LIB_DIR} ${GPERFTOOLS_LIB_DIR})

###############################################################################
# target
add_executable(${SERVER} server.cpp)
add_executable(${CLIENT} client.cpp)

###############################################################################
# library depend
target_link_libraries(${SERVER} ${LIBUDT_LIB_NAME} ${LOG4CPLUS_LIB_NAME} ${GPERFTOOLS_LIB_NAME} pthread)
target_link_libraries(${CLIENT} ${LIBUDT_LIB_NAME} ${LOG4CPLUS_LIB_NAME} ${GPERFTOOLS_LIB_NAME} pthread)

###############################################################################
# output dir
set(LIBUDT_TEST_OUTPUT_DIR ${OUTPUT_DIR}/test/udt)
MakeDir(${LIBUDT_TEST_OUTPUT_DIR})

set_target_properties(${SERVER} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${LIBUDT_TEST_OUTPUT_DIR})
set_target_properties(${CLIENT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${LIBUDT_TEST_OUTPUT_DIR})
