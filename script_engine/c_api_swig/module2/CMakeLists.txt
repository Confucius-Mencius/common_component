###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

set(LIB_NAME c_api_swig_module2)

# TODO 文件MD5判断，避免每次都生成新文件编译
execute_process(COMMAND ${SWIG_BIN_DIR}/${SWIG_BIN_NAME} -c++ -lua -o ${LIB_NAME}.cpp c_api.i WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

###############################################################################
# lib version
set(BUILD_VERSION 1.0.0)
CreateVersionFile(${CMAKE_CURRENT_SOURCE_DIR} ${PROJ_NAME} ${LIB_NAME})

###############################################################################
# include/link dir
include_directories(${LUA_INC_DIR})
link_directories(${LUA_LIB_DIR} ${GPERFTOOLS_LIB_DIR})

###############################################################################
# target
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} CUR_DIR_SRCS)
add_library(${LIB_NAME} SHARED ${HEADERS} ${CUR_DIR_SRCS})

###############################################################################
# library dependence
target_link_libraries(${LIB_NAME} ${LUA_LIB_NAME} ${GPERFTOOLS_LIB_NAME})

###############################################################################
# output dir
set_target_properties(${LIB_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})

###############################################################################
# version
set_target_properties(${LIB_NAME} PROPERTIES VERSION ${BUILD_VERSION} SOVERSION 1)
