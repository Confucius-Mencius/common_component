#ifndef APP_FRAME_SRC_CONF_XPATH_DEFINE_H_
#define APP_FRAME_SRC_CONF_XPATH_DEFINE_H_

namespace app_frame
{
const char ENABLE_CPU_PROFILING_XPATH[] = "/conf/gperftools/enable_cpu_profiling";
const char ENABLE_MEM_PROFILING_XPATH[] = "/conf/gperftools/enable_mem_profiling";
const char RELEASE_FREE_MEM_XPATH[] = "/conf/gperftools/release_free_mem";
const char GLOBAL_LOGIC_SO_XPATH[] = "/conf/global_logic_so";
const char TCP_ADDR_PORT_XPATH[] = "/conf/tcp/addr_port";
const char TCP_CONN_COUNT_LIMIT_XPATH[] = "/conf/tcp/conn_count_limit";
const char TCP_INACTIVE_CONN_CHECK_INTERVAL_SEC_XPATH[] = "/conf/tcp/inactive_conn/check_interval/sec";
const char TCP_INACTIVE_CONN_CHECK_INTERVAL_USEC_XPATH[] = "/conf/tcp/inactive_conn/check_interval/usec";
const char TCP_INACTIVE_CONN_LIFE_XPATH[] = "/conf/tcp/inactive_conn/life";
const char TCP_STORM_INTERVAL_XPATH[] = "/conf/tcp/storm_interval";
const char TCP_STORM_RECV_COUNT_XPATH[] = "/conf/tcp/storm_recv_count";
const char TCP_THREAD_COUNT_XPATH[] = "/conf/tcp/thread_count";
const char TCP_LOCAL_LOGIC_SO_XPATH[] = "/conf/tcp/local_logic_so";
const char TCP_LOGIC_SO_XPATH[] = "/conf/tcp/logic_so_group/logic_so";
const char HTTP_ADDR_PORT_XPATH[] = "/conf/http/http_addr_port";
const char HTTPS_ADDR_PORT_XPATH[] = "/conf/http/https_addr_port";
const char HTTPS_CERTIFICATE_CHAIN_FILE_PATH_XPATH[] = "/conf/http/https_certificate_chain_file_path";
const char HTTPS_PRIVATE_KEY_FILE_PATH_XPATH[] = "/conf/http/https_private_key_file_path";
const char HTTP_CONN_COUNT_LIMIT_XPATH[] = "/conf/http/conn_count_limit";
const char HTTP_MAX_HEADER_SIZE_XPATH[] = "/conf/http/max_header_size";
const char HTTP_MAX_BODY_SIZE_XPATH[] = "/conf/http/max_body_size";
const char HTTP_CONN_TIMEOUT_SEC_XPATH[] = "/conf/http/conn_timeout/sec";
const char HTTP_CONN_TIMEOUT_USEC_XPATH[] = "/conf/http/conn_timeout/usec";
const char HTTP_THREAD_COUNT_XPATH[] = "/conf/http/thread_count";
const char HTTP_LOCAL_LOGIC_SO_XPATH[] = "/conf/http/local_logic_so";
const char HTTP_LOGIC_SO_XPATH[] = "/conf/http/logic_so_group/logic_so";
const char HTTP_PRINT_PARSED_INFO_XPATH[] = "/conf/http/print_parsed_info";
const char HTTP_DECODE_URI_XPATH[] = "/conf/http/decode_uri";
const char HTTP_CONTENT_TYPE_XPATH[] = "/conf/http/content_type";
const char HTTP_NO_CACHE_XPATH[] = "/conf/Http/no_cache";
const char HTTP_FLASH_CROSS_DOMAIN_PATH_XPATH[] = "/conf/http/flash_cross_domain_path";
const char HTTP_FILE_UPLOAD_PATH_XPATH[] = "/conf/http/file_upload_path";
const char HTTP_FILE_DOWNLOAD_PATH_XPATH[] = "/conf/http/file_download_path";
const char HTTP_FILE_STORAGE_DIR_XPATH[] = "/conf/http/file_storage_dir";
const char UDP_ADDR_PORT_XPATH[] = "/conf/udp/addr_port";
const char UDP_INACTIVE_CONN_CHECK_INTERVAL_SEC_XPATH[] = "/conf/udp/inactive_conn/check_interval/sec";
const char UDP_INACTIVE_CONN_CHECK_INTERVAL_USEC_XPATH[] = "/conf/udp/inactive_conn/check_interval/usec";
const char UDP_INACTIVE_CONN_LIFE_XPATH[] = "/conf/udp/inactive_conn/life";
const char UDP_DO_CHECKSUM_XPATH[] = "/conf/udp/do_checksum";
const char UDP_MAX_MSG_BODY_LEN_XPATH[] = "/conf/udp/max_msg_body_len";
const char UDP_THREAD_COUNT_XPATH[] = "/conf/udp/thread_count";
const char UDP_LOCAL_LOGIC_SO_XPATH[] = "/conf/udp/local_logic_so";
const char UDP_LOGIC_SO_XPATH[] = "/conf/udp/logic_so_group/logic_so";
const char PEER_NEED_REPLY_MSG_CHECK_INTERVAL_XPATH[] = "/conf/peer/need_reply_msg_check_interval";
const char PEER_TCP_CONN_INTERVAL_SEC_XPATH[] = "/conf/peer/tcp/conn_interval/sec";
const char PEER_TCP_CONN_INTERVAL_USEC_XPATH[] = "/conf/peer/tcp/conn_interval/usec";
const char PEER_HTTP_CONN_TIMEOUT_XPATH[] = "/conf/peer/http/conn_timeout";
const char PEER_HTTP_CONN_MAX_RETRY_XPATH[] = "/conf/peer/http/conn_max_retry";
const char WORK_THREAD_COUNT_XPATH[] = "/conf/work_thread_count";
const char WORK_LOCAL_LOGIC_SO_XPATH[] = "/conf/work_local_logic_so";
const char WORK_LOGIC_SO_XPATH[] = "/conf/work_logic_so_group/work_logic_so";
const char IO_TO_WORK_TQ_SIZE_LIMIT_XPATH[] = "/conf/io_to_work_tq_size_limit";
const char BURDEN_THREAD_COUNT_XPATH[] = "/conf/burden_thread_count";
const char BURDEN_LOCAL_LOGIC_SO_XPATH[] = "/conf/burden_local_logic_so";
const char BURDEN_LOGIC_SO_XPATH[] = "/conf/burden_logic_so_group/burden_logic_so";
}

#endif // APP_FRAME_SRC_CONF_XPATH_DEFINE_H_
