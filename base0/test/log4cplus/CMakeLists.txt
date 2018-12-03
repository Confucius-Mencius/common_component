###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

###############################################################################
# output dir
set(LOG4CPLUS_TEST_OUTPUT_DIR ${OUTPUT_DIR}/test/log4cplus)
MakeDir(${LOG4CPLUS_TEST_OUTPUT_DIR})

execute_process(COMMAND cp -f SQFront.log.properties ${LOG4CPLUS_TEST_OUTPUT_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

##
set(DAILY_LOG_TEST_NAME daily_log_test)

###############################################################################
# include/link dir
include_directories(${LOG4CPLUS_INC_DIR} ${OPENSSL_INC_DIR})
link_directories(${LOG4CPLUS_LIB_DIR} ${OPENSSL_LIB_DIR} ${GPERFTOOLS_LIB_DIR})

###############################################################################
# target and linked library
add_executable(${DAILY_LOG_TEST_NAME} daily_log_test.cpp)

target_link_libraries(${DAILY_LOG_TEST_NAME} ${LOG4CPLUS_LIB_NAME} ${OPENSSL_LIB_NAME} ${GPERFTOOLS_LIB_NAME} pthread)

set_target_properties(${DAILY_LOG_TEST_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${LOG4CPLUS_TEST_OUTPUT_DIR})
