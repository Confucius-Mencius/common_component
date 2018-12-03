###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

###############################################################################
# output dir
set(LIBEVENT_TEST_OUTPUT_DIR ${OUTPUT_DIR}/test/libevent)
MakeDir(${LIBEVENT_TEST_OUTPUT_DIR})

###############################################################################
# sub dir
add_subdirectory(signal)
add_subdirectory(timer)
add_subdirectory(rot13_server)
add_subdirectory(multi_thread)
add_subdirectory(multi_thread_io)
add_subdirectory(http_client)
add_subdirectory(http_server)
add_subdirectory(http_multithread_server)
add_subdirectory(connect_broken)
add_subdirectory(http_server_bug)
add_subdirectory(http_chunk)

# 下面几个libevent2.1版本才支持,2.0版本不支持
add_subdirectory(https_client)
add_subdirectory(https_server)
add_subdirectory(http_connect)

add_subdirectory(http_server_form)
