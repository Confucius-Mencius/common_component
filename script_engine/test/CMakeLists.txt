###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

set(TEST_NAME ${PROJ_NAME}_test)
set(EXE_NAME ${TEST_NAME})

###############################################################################
# include/link dir
include_directories(${PROJ_INC_DIR} ${C_API_TOLUAPP_INC_DIR} ${BASE_INC_DIR} ${TOLUAPP_INC_DIR} ${LUA_INC_DIR}
        ${GTEST_INC_DIR})
link_directories(${BASE_LIB_DIR} ${TOLUAPP_LIB_DIR} ${LUA_LIB_DIR} ${GTEST_LIB_DIR})

###############################################################################
# target
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${PROJ_INC_DIR}/*.h)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} CUR_DIR_SRCS)
add_executable(${EXE_NAME} ${HEADERS} ${CUR_DIR_SRCS})

###############################################################################
# library dependence
target_link_libraries(${EXE_NAME} ${C_API_TOLUAPP_LIB_NAME} ${BASE_LIB_NAME} ${TOLUAPP_LIB_NAME} ${LUA_LIB_NAME}
        ${GTEST_LIB_NAME} pthread)

###############################################################################
# output dir
set(TEST_OUTPUT_DIR ${OUTPUT_DIR}/test)
MakeDir(${TEST_OUTPUT_DIR})

set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIR})

###############################################################################
# add test
add_test(NAME ${TEST_NAME} WORKING_DIRECTORY ${TEST_OUTPUT_DIR} COMMAND ${EXE_NAME} ${GTEST_EXE_OPTS}${TEST_NAME}.xml)

###############################################################################
# copy files for testing
execute_process(COMMAND cp -rf data ${TEST_OUTPUT_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
