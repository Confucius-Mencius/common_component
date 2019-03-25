###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

set(EXE_NAME ${PROJ_NAME})

###############################################################################
# lib version
set(BUILD_VERSION 1.0.0)
CreateVersionFile(${CMAKE_CURRENT_SOURCE_DIR} ${PROJ_NAME} ${EXE_NAME})

###############################################################################
# include/link dir
include_directories(${PROJ_INC_DIR} ${LOG_ENGINE_INC_DIR} ${CONF_CENTER_INC_DIR}
        ${THREAD_CENTER_INC_DIR} ${APP_FRAME_INC_DIR} ${BASE_INC_DIR} ${GFLAGS_INC_DIR}
        ${LIBEVENT_INC_DIR} ${LOG4CPLUS_INC_DIR})
link_directories(${BASE_LIB_DIR} ${GFLAGS_LIB_DIR} ${LIBEVENT_LIB_DIR} ${LOG4CPLUS_LIB_DIR} ${GPERFTOOLS_LIB_DIR})

###############################################################################
# target
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${PROJ_INC_DIR}/*.h)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} CUR_DIR_SRCS)
add_executable(${EXE_NAME} ${HEADERS} ${CUR_DIR_SRCS})

###############################################################################
# library dependence
target_link_libraries(${EXE_NAME} ${BASE_LIB_NAME} ${GFLAGS_LIB_NAME} ${LIBEVENT_LIB_NAME} ${LOG4CPLUS_LIB_NAME}
        pthread ${GPERFTOOLS_LIB_NAME})

###############################################################################
# output dir
set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

###############################################################################
# install exe
InstallExe(${EXE_NAME} lib)
