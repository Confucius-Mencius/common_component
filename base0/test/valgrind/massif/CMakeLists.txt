###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

set(EXE_NAME massif_test)

###############################################################################
# target and linked library
file(GLOB HEADERS *.h)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} CUR_DIR_SRCS)
add_executable(${EXE_NAME} ${HEADERS} ${CUR_DIR_SRCS})

set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${VALGRIND_TEST_OUTPUT_DIR})

execute_process(COMMAND chmod +x massif_test.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
execute_process(COMMAND cp massif_test.sh ${VALGRIND_TEST_OUTPUT_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
