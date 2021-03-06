###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

set(LIB_NAME ${PROJ_NAME})

###############################################################################
# lib version
set(BUILD_VERSION 1.0.0)
CreateVersionFile(${CMAKE_CURRENT_SOURCE_DIR} ${PROJ_NAME} ${LIB_NAME})

###############################################################################
# include/link dir
include_directories(${PROJ_INC_DIR} ${LOG_ENGINE_INC_DIR} ${PROTO_MSG_CODEC_INC_DIR}
        ${TRANS_CENTER_INC_DIR} ${TIMER_AXIS_INC_DIR} ${BASE_INC_DIR} ${LIBEVENT_INC_DIR}
        ${HTTP_PARSER_INC_DIR} ${OPENSSL_INC_DIR} ${LOG4CPLUS_INC_DIR} ${GPERFTOOLS_INC_DIR})
link_directories(${BASE_LIB_DIR} ${HTTP_PARSER_LIB_DIR} ${LIBEVENT_LIB_DIR}
        ${OPENSSL_LIB_DIR} ${GPERFTOOLS_LIB_DIR})

###############################################################################
# target
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${PROJ_INC_DIR}/*.h)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} CUR_DIR_SRCS)
add_library(${LIB_NAME} SHARED ${HEADERS} ${CUR_DIR_SRCS})

###############################################################################
# library dependence
target_link_libraries(${LIB_NAME} ${BASE_LIB_NAME} ${HTTP_PARSER_LIB_NAME}
        ${LIBEVENT_LIB_NAME} ${OPENSSL_LIB_NAME} ${GPERFTOOLS_LIB_NAME})

###############################################################################
# output dir
set_target_properties(${LIB_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})

###############################################################################
# version
set_target_properties(${LIB_NAME} PROPERTIES VERSION ${BUILD_VERSION} SOVERSION 1)

###############################################################################
# install lib and include files
InstallLib(${LIB_NAME} lib)
InstallHeaderFiles(${PROJ_INC_DIR} include)
