###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

set(TEST_NAME ${PROJ_NAME}_test)
set(EXE_NAME ${TEST_NAME})

###############################################################################
# include/link dir
include_directories(${PROJ_INC_DIR} ${BASE_INC_DIR} ${LOG4CPLUS_INC_DIR}
        ${GTEST_INC_DIR} ${GPERFTOOLS_INC_DIR})
link_directories(${BASE_LIB_DIR} ${LOG4CPLUS_LIB_DIR} ${GTEST_LIB_DIR}
        ${GPERFTOOLS_LIB_DIR})

###############################################################################
# target
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${PROJ_INC_DIR}/*.h)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} CUR_DIR_SRCS)
add_executable(${EXE_NAME} ${HEADERS} ${CUR_DIR_SRCS})

###############################################################################
# library dependence
target_link_libraries(${EXE_NAME} ${BASE_LIB_NAME} ${LOG4CPLUS_LIB_NAME}
        ${GTEST_LIB_NAME} crypto pthread ${GPERFTOOLS_LIB_NAME})

###############################################################################
# output dir
set(TEST_OUTPUT_DIR ${OUTPUT_DIR}/test)
MakeDir(${TEST_OUTPUT_DIR})
MakeDir(${TEST_OUTPUT_DIR}/logs)

set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIR})

###############################################################################
# add test
add_test(NAME ${TEST_NAME} WORKING_DIRECTORY ${TEST_OUTPUT_DIR} COMMAND ${EXE_NAME} ${GTEST_EXE_OPTS}${TEST_NAME}.xml)

###############################################################################
# copy log conf file for testing
execute_process(COMMAND cp -f test_log_conf.properties ${TEST_OUTPUT_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
