###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

set(JSON2PB_TEST json2pb_test)

execute_process(COMMAND /opt/third_party/protobuf/bin/protoc --cpp_out=. test.proto WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

###############################################################################
# include/link dir
include_directories(${JSON2PB_INC_DIR} ${PROTOBUF_INC_DIR})
link_directories(${JSON2PB_LIB_DIR} ${JANSSON_LIB_DIR} ${PROTOBUF_LIB_DIR} ${GPERFTOOLS_LIB_DIR})

###############################################################################
# target
aux_source_directory(. CUR_DIR_SRCS)
add_executable(${JSON2PB_TEST} ${CUR_DIR_SRCS})

###############################################################################
# library depend
target_link_libraries(${JSON2PB_TEST} ${JSON2PB_LIB_NAME} ${JANSSON_LIB_NAME} ${PROTOBUF_LIB_NAME} ${GPERFTOOLS_LIB_NAME} pthread)

###############################################################################
# output dir
set(JSON2PB_TEST_OUTPUT_DIR ${OUTPUT_DIR}/test/json2pb)
MakeDir(${JSON2PB_TEST_OUTPUT_DIR})

execute_process(COMMAND cp test.json ${JSON2PB_TEST_OUTPUT_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${JSON2PB_TEST} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${JSON2PB_TEST_OUTPUT_DIR})
