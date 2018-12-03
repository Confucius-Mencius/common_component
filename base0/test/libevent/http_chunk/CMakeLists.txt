###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

set(EXE_NAME libevent_http_chunk)

###############################################################################
# include/link dir
include_directories(${LIBEVENT_INC_DIR})
link_directories(${LIBEVENT_LIB_DIR} ${OPENSSL_LIB_DIR} ${GPERFTOOLS_LIB_DIR})

###############################################################################
# target and linked library
file(GLOB HEADERS *.h)
aux_source_directory(../http_client CUR_DIR_SRCS)
add_executable(${EXE_NAME} ${HEADERS} ${CUR_DIR_SRCS})

target_link_libraries(${EXE_NAME} ${LIBEVENT_LIB_NAME} ${GPERFTOOLS_LIB_NAME})

set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${LIBEVENT_TEST_OUTPUT_DIR})
