###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

option(THREAD "zookeeper mutilthread lib" ON)

set(BASE_LIB_NAME ${PROJ_NAME})
set(BASE_LIB_DIR ${OUTPUT_DIR})

set(TEST_NAME ${BASE_LIB_NAME}_test)
set(EXE_NAME ${TEST_NAME})

# libzookeeper_mt.so
add_definitions(-DTHREADED)

###############################################################################
# include/link dir
include_directories(${PROJ_INC_DIR} ${LIBUUID_INC_DIR} ${CURL_INC_DIR} ${LIBEVENT_INC_DIR} ${JSONCPP_INC_DIR}
        ${MONGO_CXX_DRIVER_INC_DIR} ${MONGO_C_DRIVER_INC_DIR} ${LIBBSON_INC_DIR}
        ${ZOOKEEPER_INC_DIR} ${ZEROMQ_INC_DIR} ${LOG4CPLUS_INC_DIR} ${BOOST_INC_DIR} ${OPENSSL_INC_DIR}
        ${LEVELDB_INC_DIR} ${GTEST_INC_DIR})
link_directories(${BASE_LIB_DIR} ${LIBUUID_LIB_DIR} ${CURL_LIB_DIR} ${LIBEVENT_LIB_DIR} ${JSONCPP_LIB_DIR}
        ${MONGO_CXX_DRIVER_LIB_DIR} ${MONGO_C_DRIVER_LIB_DIR} ${LIBBSON_LIB_DIR}
        ${ZOOKEEPER_LIB_DIR} ${ZEROMQ_LIB_DIR} ${OPENCC_LIB_DIR} ${LOG4CPLUS_LIB_DIR} ${BOOST_LIB_DIR} ${OPENSSL_LIB_DIR}
        ${LEVELDB_LIB_DIR} ${GTEST_LIB_DIR} ${GPERFTOOLS_LIB_DIR})

###############################################################################
# target and linked library
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${PROJ_INC_DIR}/*.h)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} CUR_DIR_SRCS)

if (COMPILER_SUPPORTS_CXX11)
    aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/cpp11 CPP11_TEST_SRCS)
endif ()

add_executable(${EXE_NAME} ${HEADERS} ${CUR_DIR_SRCS} ${CPP11_TEST_SRCS})

target_link_libraries(${EXE_NAME} ${BASE_LIB_NAME} ${LIBUUID_LIB_NAME} ${CURL_LIB_NAME} ${LIBEVENT_LIB_NAME}
        ${JSONCPP_LIB_NAME} ${MONGO_CXX_DRIVER_LIB_NAME} ${MONGO_C_DRIVER_LIB_NAME}
        ${LIBBSON_LIB_NAME} ${ZOOKEEPER_LIB_NAME} ${ZEROMQ_LIB_NAME} ${LOG4CPLUS_LIB_NAME} ${BOOST_LIB_NAME}
        ${OPENSSL_LIB_NAME} ${LEVELDB_LIB_NAME} ${GTEST_LIB_NAME} pthread ${GPERFTOOLS_LIB_NAME})

###############################################################################
# output dir
set(TEST_OUTPUT_DIR ${OUTPUT_DIR}/test)
MakeDir(${TEST_OUTPUT_DIR})

set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIR})

###############################################################################
# add test
add_test(NAME ${TEST_NAME} WORKING_DIRECTORY ${TEST_OUTPUT_DIR} COMMAND ${EXE_NAME} ${GTEST_EXE_OPTS}${TEST_NAME}.xml)

execute_process(COMMAND cp -f rsa_public_key.pem rsa_private_key.pem ${TEST_OUTPUT_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
