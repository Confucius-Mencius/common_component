###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

set(LIB_NAME ${C_API_TOLUAPP_LIB_NAME})

# TODO 文件MD5判断，避免每次都生成新文件编译
execute_process(COMMAND ${TOLUAPP_BIN_DIR}/${TOLUAPP_BIN_NAME} -S -n ${C_API_TOLUAPP_LIB_NAME}
        -o ${C_API_TOLUAPP_LIB_NAME}.cpp -H ${C_API_TOLUAPP_LIB_NAME}.h c_api.pkg WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

###############################################################################
# lib version
set(BUILD_VERSION 1.0.0)
CreateVersionFile(${CMAKE_CURRENT_SOURCE_DIR} ${PROJ_NAME} ${LIB_NAME})

###############################################################################
# include/link dir
include_directories(${PROJ_INC_DIR} ${TOLUAPP_INC_DIR} ${LUA_INC_DIR})
link_directories(${TOLUAPP_LIB_DIR} ${LUA_LIB_DIR} ${GPERFTOOLS_LIB_DIR})

###############################################################################
# target
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} CUR_DIR_SRCS)
add_library(${LIB_NAME} SHARED ${HEADERS} ${CUR_DIR_SRCS})

###############################################################################
# library dependence
target_link_libraries(${LIB_NAME} ${TOLUAPP_LIB_NAME} ${LUA_LIB_NAME} ${GPERFTOOLS_LIB_NAME})

###############################################################################
# output dir
set_target_properties(${LIB_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})

###############################################################################
# version
set_target_properties(${LIB_NAME} PROPERTIES VERSION ${BUILD_VERSION} SOVERSION 1)
