###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

set(BASE_LIB_NAME ${PROJ_NAME})
set(BASE_LIB_DIR ${OUTPUT_DIR})

set(TEST_NAME ${BASE_LIB_NAME}_test)
set(EXE_NAME ${TEST_NAME})

###############################################################################
# include/link dir
include_directories(${PROJ_INC_DIR} ${LIBUUID_INC_DIR} ${SPDLOG_INC_DIR} ${GTEST_INC_DIR})
link_directories(${BASE_LIB_DIR} ${LIBUUID_LIB_DIR} ${GTEST_LIB_DIR} ${GPERFTOOLS_LIB_DIR})

###############################################################################
# target and linked library
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${PROJ_INC_DIR}/*.h)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} CUR_DIR_SRCS)

add_executable(${EXE_NAME} ${HEADERS} ${CUR_DIR_SRCS})

target_link_libraries(${EXE_NAME} ${BASE_LIB_NAME} ${LIBUUID_LIB_NAME} ${GTEST_LIB_NAME}
        pthread ${GPERFTOOLS_LIB_NAME})

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
# copy files for testing
execute_process(COMMAND cp -rf data ${TEST_OUTPUT_DIR}/ WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
