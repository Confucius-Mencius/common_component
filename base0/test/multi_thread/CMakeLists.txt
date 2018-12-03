###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

###############################################################################
# output dir
set(MULTI_THREAD_TEST_OUTPUT_DIR ${OUTPUT_DIR}/test/multi_thread)
MakeDir(${MULTI_THREAD_TEST_OUTPUT_DIR})

set(EXE_NAME multi_thread_test)

###############################################################################
# target and linked library
add_executable(${EXE_NAME} sub_thread.h sub_thread_test.cpp)
target_link_libraries(${EXE_NAME} pthread)

set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MULTI_THREAD_TEST_OUTPUT_DIR})
