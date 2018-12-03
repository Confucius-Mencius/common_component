###############################################################################
# author: BrentHuang (guang11cheng@qq.com)
###############################################################################

###############################################################################
# output dir
set(LIBEVHTP_TEST_OUTPUT_DIR ${OUTPUT_DIR}/test/libevhtp)
MakeDir(${LIBEVHTP_TEST_OUTPUT_DIR})

###############################################################################
# sub dir
add_subdirectory(http_server)
add_subdirectory(http_server_form)
add_subdirectory(http_client)
