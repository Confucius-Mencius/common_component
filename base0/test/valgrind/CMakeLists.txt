###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

###############################################################################
# output dir
set(VALGRIND_TEST_OUTPUT_DIR ${OUTPUT_DIR}/test/valgrind)
MakeDir(${VALGRIND_TEST_OUTPUT_DIR})

add_subdirectory(memcheck)
add_subdirectory(massif)
add_subdirectory(callgrind)
