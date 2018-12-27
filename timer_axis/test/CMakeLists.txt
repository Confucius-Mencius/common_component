###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

set(TEST_NAME ${PROJ_NAME}_test)
set(EXE_NAME ${TEST_NAME})

###############################################################################
# include/link dir
include_directories(${PROJ_INC_DIR} ${LOG_ENGINE_INC_DIR} ${BASE_INC_DIR} ${LIBEVENT_INC_DIR}
        ${LOG4CPLUS_INC_DIR} ${GTEST_INC_DIR})
link_directories(${BASE_LIB_DIR} ${LIBEVENT_LIB_DIR} ${OPENSSL_LIB_DIR} ${LOG4CPLUS_LIB_DIR} ${GTEST_LIB_DIR})

###############################################################################
# target
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${PROJ_INC_DIR}/*.h)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} CUR_DIR_SRCS)
add_executable(${EXE_NAME} ${HEADERS} ${CUR_DIR_SRCS})

###############################################################################
# library dependence
target_link_libraries(${EXE_NAME} ${BASE_LIB_NAME} ${LIBEVENT_LIB_NAME} ${OPENSSL_LIB_NAME} ${LOG4CPLUS_LIB_NAME} ${GTEST_LIB_NAME} pthread)

###############################################################################
# output dir
set(TEST_OUTPUT_DIR ${OUTPUT_DIR}/test)
MakeDir(${TEST_OUTPUT_DIR})

set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIR})

###############################################################################
# add test
add_test(NAME ${TEST_NAME} WORKING_DIRECTORY ${TEST_OUTPUT_DIR} COMMAND ${EXE_NAME} ${GTEST_EXE_OPTS}${TEST_NAME}.xml)
