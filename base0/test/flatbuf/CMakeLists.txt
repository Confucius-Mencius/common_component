###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

set(FLATBUF_TEST flatbuf_test)

set(FLATC ${FLATBUF_BIN_DIR}/${FLATBUF_BIN_NAME})
execute_process(COMMAND ${FLATC} --cpp --gen-mutable msg.fbs WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

###############################################################################
# include/link dir
include_directories(${FLATBUF_INC_DIR})
link_directories(${FLATBUF_LIB_DIR} ${OPENSSL_LIB_DIR} ${GPERFTOOLS_LIB_DIR})

###############################################################################
# target
add_executable(${FLATBUF_TEST} flatbuf_test.cpp)

###############################################################################
# library depend
target_link_libraries(${FLATBUF_TEST} ${FLATBUF_LIB_NAME} ${OPENSSL_LIB_NAME} ${GPERFTOOLS_LIB_NAME} pthread)

###############################################################################
# output dir
set(FLATBUF_TEST_OUTPUT_DIR ${OUTPUT_DIR}/test/flatbuf)
MakeDir(${FLATBUF_TEST_OUTPUT_DIR})

set_target_properties(${FLATBUF_TEST} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${FLATBUF_TEST_OUTPUT_DIR})
