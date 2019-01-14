###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

###############################################################################
set(CMAKE_VERBOSE_MAKEFILE ON)

###############################################################################
# c++11
if (CXX_LANGUAGE)
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
endif ()

###############################################################################
# function: SetCompileFlags
# treat_warn_as_error: TRUR/FALSE
function(SetCompileFlags treat_warn_as_error)
    if (${treat_warn_as_error})
        # set(COMPILE_FLAGS "-g -Wall -Wextra -Werror -Wno-deprecated-declarations -Wno-unused-function -Wno-unused-result -Wno-unused-variable -Wno-unused-but-set-variable")
        set(COMPILE_FLAGS "-g -Wall -Wextra -Werror -Wno-deprecated-declarations")
    else (${treat_warn_as_error})
        set(COMPILE_FLAGS "-g -Wall -Wextra")
    endif ()

    # set(CXX11_FLAG "-std=c++11")
    set(CXX11_FLAG "-std=gnu++11")

    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        set(OPTIMIZE_FLAG "-O0")

        set(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} ${OPTIMIZE_FLAG} ${COMPILE_FLAGS}" PARENT_SCOPE)

        if (COMPILER_SUPPORTS_CXX11)
            # set(CMAKE_CXX_STANDARD 11)
            set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} ${OPTIMIZE_FLAG} ${COMPILE_FLAGS} ${CXX11_FLAG}" PARENT_SCOPE)
        else (COMPILER_SUPPORTS_CXX11)
            set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} ${OPTIMIZE_FLAG} ${COMPILE_FLAGS}" PARENT_SCOPE)
        endif ()
    elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release")
        set(OPTIMIZE_FLAG "-O2")

        set(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} ${OPTIMIZE_FLAG} ${COMPILE_FLAGS}" PARENT_SCOPE)

        if (COMPILER_SUPPORTS_CXX11)
            # set(CMAKE_CXX_STANDARD 11)
            set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} ${OPTIMIZE_FLAG} ${COMPILE_FLAGS} ${CXX11_FLAG}" PARENT_SCOPE)
        else (COMPILER_SUPPORTS_CXX11)
            set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} ${OPTIMIZE_FLAG} ${COMPILE_FLAGS}" PARENT_SCOPE)
        endif ()

        add_definitions(-DNDEBUG) # 定义一个宏，在c++代码中可以使用
    endif ()
endfunction()

###############################################################################
# test and gtest
include(CTest)
set(GTEST_EXE_OPTS --gtest_shuffle --gtest_color=yes --gtest_output=xml:report/)

###############################################################################
# function: SetOutputDir
function(SetOutputDir output_base_dir proj)
    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        set(OUTPUT_DIR ${output_base_dir}/debug/${proj} PARENT_SCOPE)
        set(OUTPUT_DIR_ ${output_base_dir}/debug/${proj})
    elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release")
        set(OUTPUT_DIR ${output_base_dir}/release/${proj} PARENT_SCOPE)
        set(OUTPUT_DIR_ ${output_base_dir}/release/${proj})
    endif ()

    if (NOT EXISTS ${OUTPUT_DIR_})
        file(MAKE_DIRECTORY ${OUTPUT_DIR_})
    endif ()
endfunction()


###############################################################################
# function: MakeDir
function(MakeDir dir)
    if (NOT EXISTS ${dir})
        file(MAKE_DIRECTORY ${dir})
    endif ()
endfunction()

###############################################################################
# function: CreateVersionFile
function(CreateVersionFile dir proj_name lib_name)
    if (NOT EXISTS ${dir})
        file(MAKE_DIRECTORY ${dir})
    endif ()

    STRING(TOUPPER ${proj_name} UPPER_PROJ_NAME)
    STRING(TOUPPER ${lib_name} UPPER_LIB_NAME)

    FILE(WRITE ${dir}/version.h
            "\#ifndef " ${UPPER_PROJ_NAME} "_SRC_VERSION_H_ \n"
            "\#define " ${UPPER_PROJ_NAME} "_SRC_VERSION_H_\n\n"
            "\#define " ${UPPER_PROJ_NAME} "_VERSION \"" ${BUILD_VERSION} "\"\n\n"
            "\#endif // " ${UPPER_PROJ_NAME} "_SRC_VERSION_H_\n\n")
endfunction()

###############################################################################
# function: InstallExe
# target_dir: 如果是相对目录，则是相对于CMAKE_INSTALL_PREFIX
function(InstallExe exe_name target_dir)
    install(TARGETS ${exe_name} RUNTIME DESTINATION ${target_dir})
endfunction()

###############################################################################
# function: InstallLib
# target_dir: 如果是相对目录，则是相对于CMAKE_INSTALL_PREFIX
function(InstallLib lib_name target_dir)
    install(TARGETS ${lib_name} LIBRARY DESTINATION ${target_dir})
endfunction()

###############################################################################
# function: InstallHeaderFiles
# target_dir: 如果是相对目录，则是相对于CMAKE_INSTALL_PREFIX
function(InstallHeaderFiles src_dir target_dir)
    install(DIRECTORY ${src_dir}/ DESTINATION ${target_dir}
            FILES_MATCHING PATTERN *.h PATTERN ".svn" EXCLUDE)
endfunction()

###############################################################################
# function: ShowBuildOptions
function(ShowBuildOptions)
    message("cmake build type: " ${CMAKE_BUILD_TYPE})

    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        message("cmake cflags: " ${CMAKE_C_FLAGS_DEBUG})
        message("cmake cxxflags: " ${CMAKE_CXX_FLAGS_DEBUG})
    elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release")
        message("cmake cflags: " ${CMAKE_C_FLAGS_RELEASE})
        message("cmake cxxflags: " ${CMAKE_CXX_FLAGS_RELEASE})
    endif ()

    message("output dir: " ${OUTPUT_DIR})
    message("install prefix: " ${CMAKE_INSTALL_PREFIX})
endfunction()

###############################################################################
# Detecting linux distro
message("os: ${CMAKE_SYSTEM_NAME}")

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    # Detect Linux distribution (if possible)
    if ((EXISTS /usr/bin/yum) AND (NOT EXISTS /usr/bin/lsb_release))
        execute_process(COMMAND sudo yum install redhat-lsb-core -y)
    endif ()

    execute_process(COMMAND "/usr/bin/lsb_release" "-is"
            TIMEOUT 4
            OUTPUT_VARIABLE LINUX_DISTRO
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    message("linux distro: ${LINUX_DISTRO}")
endif ()

###############################################################################
# 下列include需要先定义THIRD_PARTY_DIR
include(${THIRD_PARTY_DIR}/cmake/common_define.cmake)

# 下列几个库的include和lib路径有修改，需要覆盖common_define.cmake中的值
set(LIBXML2_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/libxml2/include/libxml2)
set(IMAGE_MAGICK_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/image_magick/include/ImageMagick-6)
set(ZOOKEEPER_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/zookeeper/include/zookeeper)
set(MYSQL_CLIENT_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/mysql_client/include/mysql)
set(MYSQL_CLIENT_LIB_DIR ${THIRD_PARTY_INSTALL_PREFIX}/mysql_client/lib/mysql)
# set(LIBBSON_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/libbson/include/libbson-1.0)
set(MONGO_C_DRIVER_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/mongo_c_driver/include/libmongoc-1.0)
set(MONGO_CXX_DRIVER_INC_DIR ${THIRD_PARTY_INSTALL_PREFIX}/mongo_cxx_driver/include/mongocxx/v_noabi ${MONGO_C_DRIVER_INC_DIR} ${THIRD_PARTY_INSTALL_PREFIX}/mongo_cxx_driver/include/bsoncxx/v_noabi ${LIBBSON_INC_DIR})
set(LIBWEBSOCKETS_LIB_DIR ${THIRD_PARTY_INSTALL_PREFIX}/libwebsockets/lib64)

if (${LINUX_DISTRO} MATCHES "CentOS")
    set(JSONCPP_LIB_DIR ${THIRD_PARTY_INSTALL_PREFIX}/jsoncpp/lib64)
else(${LINUX_DISTRO} MATCHES "CentOS")
    set(JSONCPP_LIB_DIR ${THIRD_PARTY_INSTALL_PREFIX}/jsoncpp/lib)
endif ()

###############################################################################
# debug时不链接gperftools
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    unset(GPERFTOOLS_INC_DIR)
    unset(GPERFTOOLS_LIB_DIR)
    unset(GPERFTOOLS_BIN_DIR)
    unset(GPERFTOOLS_LIB_NAME)
endif ()
