###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
set(THIRD_PARTY_INSTALL_PREFIX /opt/third_party/${BUILD_TYPE})

set(OPENSSL_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/openssl/include)
set(OPENSSL_LIB_DIR ${THIRD_PARTY_INSTALL_PREFIX}/openssl/lib)
set(OPENSSL_LIB_NAME crypto ssl)

set(CURL_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/curl/include)
set(CURL_LIB_DIR ${THIRD_PARTY_INSTALL_PREFIX}/curl/lib)
set(CURL_BIN_DIR ${THIRD_PARTY_INSTALL_PREFIX}/curl/bin)
set(CURL_LIB_NAME curl)
set(CURL_BIN_NAME curl)

set(GFLAGS_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/gflags/include)
set(GFLAGS_LIB_DIR ${THIRD_PARTY_INSTALL_PREFIX}/gflags/lib)
set(GFLAGS_LIB_NAME gflags)

set(GTEST_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/gtest/include)
set(GTEST_LIB_DIR ${THIRD_PARTY_INSTALL_PREFIX}/gtest/lib)
set(GTEST_LIB_NAME gmock gtest)

set(LIBUNWIND_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/libunwind/include)
set(LIBUNWIND_LIB_DIR ${THIRD_PARTY_INSTALL_PREFIX}/libunwind/lib)
set(LIBUNWIND_LIB_NAME unwind)

set(GPERFTOOLS_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/gperftools/include)
set(GPERFTOOLS_LIB_DIR ${THIRD_PARTY_INSTALL_PREFIX}/gperftools/lib)
set(GPERFTOOLS_BIN_DIR ${THIRD_PARTY_INSTALL_PREFIX}/gperftools/bin)
set(GPERFTOOLS_LIB_NAME tcmalloc profiler)
set(GPERFTOOLS_BIN_NAME pprof)

set(LIBUUID_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/libuuid/include)
set(LIBUUID_LIB_DIR ${THIRD_PARTY_INSTALL_PREFIX}/libuuid/lib)
set(LIBUUID_LIB_NAME uuid)

set(LIBEVENT_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/libevent/include)
set(LIBEVENT_LIB_DIR ${THIRD_PARTY_INSTALL_PREFIX}/libevent/lib)
set(LIBEVENT_LIB_NAME event_core event_extra event_pthreads event_openssl)

set(LIBXML2_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/libxml2/include)
set(LIBXML2_LIB_DIR ${THIRD_PARTY_INSTALL_PREFIX}/libxml2/lib)
set(LIBXML2_LIB_NAME xml2)

set(LOG4CPLUS_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/log4cplus/include)
set(LOG4CPLUS_LIB_DIR ${THIRD_PARTY_INSTALL_PREFIX}/log4cplus/lib)
set(LOG4CPLUS_BIN_DIR ${THIRD_PARTY_INSTALL_PREFIX}/log4cplus/bin)
set(LOG4CPLUS_LIB_NAME log4cplus)
set(LOG4CPLUS_BIN_NAME loggingserver)

set(HTTP_PARSER_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/http_parser/include)
set(HTTP_PARSER_LIB_DIR ${THIRD_PARTY_INSTALL_PREFIX}/http_parser/lib)
set(HTTP_PARSER_LIB_NAME http_parser)

set(PROTOBUF_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/protobuf/include)
set(PROTOBUF_LIB_DIR ${THIRD_PARTY_INSTALL_PREFIX}/protobuf/lib)
set(PROTOBUF_BIN_DIR ${THIRD_PARTY_INSTALL_PREFIX}/protobuf/bin)
set(PROTOBUF_LIB_NAME protobuf)
set(PROTOBUF_BIN_NAME protoc)

set(RAPIDJSON_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/rapidjson/include)

set(MARIADB_CONNECTOR_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/mariadb_connector/include)
set(MARIADB_CONNECTOR_LIB_DIR ${THIRD_PARTY_INSTALL_PREFIX}/mariadb_connector/lib)
set(MARIADB_CONNECTOR_LIB_NAME mariadb)

set(HIREDIS_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/hiredis/include)
set(HIREDIS_LIB_DIR ${THIRD_PARTY_INSTALL_PREFIX}/hiredis/lib)
set(HIREDIS_LIB_NAME hiredis)

set(ZOOKEEPER_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/zookeeper/include)
set(ZOOKEEPER_LIB_DIR ${THIRD_PARTY_INSTALL_PREFIX}/zookeeper/lib)
set(ZOOKEEPER_LIB_NAME zookeeper_mt)

set(AWS_SDK_CPP_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/aws_sdk_cpp/include)
set(AWS_SDK_CPP_LIB_DIR ${THIRD_PARTY_INSTALL_PREFIX}/aws_sdk_cpp/lib)
set(AWS_SDK_CPP_LIB_NAME aws-cpp-sdk-core aws-cpp-sdk-sts aws-cpp-sdk-s3)
