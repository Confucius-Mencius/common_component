#ifndef APP_FRAME_SRC_CONF_XPATH_H_
#define APP_FRAME_SRC_CONF_XPATH_H_

namespace app_frame
{
const char ENABLE_CPU_PROFILING_XPATH[] = "/conf/gperftools/enable_cpu_profiling";
const char ENABLE_HEAP_PROFILING_XPATH[] = "/conf/gperftools/enable_heap_profiling";
const char RELEASE_FREE_MEM_XPATH[] = "/conf/gperftools/release_free_mem";
const char GLOBAL_LOGIC_SO_XPATH[] = "/conf/global_logic_so";
const char WORK_THREAD_COUNT_XPATH[] = "/conf/work/thread_count";
const char WORK_COMMON_LOGIC_SO_XPATH[] = "/conf/work/common_logic_so";
const char WORK_LOGIC_SO_XPATH[] = "/conf/work/logic_so_group/logic_so";
const char BURDEN_THREAD_COUNT_XPATH[] = "/conf/burden/thread_count";
const char BURDEN_COMMON_LOGIC_SO_XPATH[] = "/conf/burden/common_logic_so";
const char BURDEN_LOGIC_SO_XPATH[] = "/conf/burden/logic_so_group/logic_so";
const char TCP_CONN_COUNT_LIMIT_XPATH[] = "/conf/tcp/conn_count_limit";
const char TCP_INACTIVE_CONN_CHECK_INTERVAL_SEC_XPATH[] = "/conf/tcp/inactive_conn/check_interval/sec";
const char TCP_INACTIVE_CONN_CHECK_INTERVAL_USEC_XPATH[] = "/conf/tcp/inactive_conn/check_interval/usec";
const char TCP_INACTIVE_CONN_LIFE_XPATH[] = "/conf/tcp/inactive_conn/life";
const char TCP_STORM_INTERVAL_XPATH[] = "/conf/tcp/storm_interval";
const char TCP_STORM_THRESHOLD_XPATH[] = "/conf/tcp/storm_threshold";
const char RAW_TCP_USE_BUFFEREVENT_XPATH[] = "/conf/tcp/raw/use_bufferevent";
const char RAW_TCP_ADDR_XPATH[] = "/conf/tcp/raw/addr";
const char RAW_TCP_PORT_XPATH[] = "/conf/tcp/raw/port";
const char RAW_TCP_THREAD_COUNT_XPATH[] = "/conf/tcp/raw/thread_count";
const char RAW_TCP_COMMON_LOGIC_SO_XPATH[] = "/conf/tcp/raw/common_logic_so";
const char RAW_TCP_LOGIC_SO_XPATH[] = "/conf/tcp/raw/logic_so_group/logic_so";
const char PROTO_TCP_USE_BUFFEREVENT_XPATH[] = "/conf/tcp/proto/use_bufferevent";
const char PROTO_DO_CHECKSUM_XPATH[] = "/conf/tcp/proto/do_checksum";
const char PROTO_MAX_MSG_BODY_LEN_XPATH[] = "/conf/tcp/proto/max_msg_body_len";
const char PROTO_PART_MSG_CHECK_INTERVAL_XPATH[] = "/conf/tcp/proto/part_msg/check_interval";
const char PROTO_PART_MSG_CONN_LIFE_XPATH[] = "/conf/tcp/proto/part_msg/conn_life";
const char PROTO_TCP_ADDR_XPATH[] = "/conf/tcp/proto/addr";
const char PROTO_TCP_PORT_XPATH[] = "/conf/tcp/proto/port";
const char PROTO_TCP_THREAD_COUNT_XPATH[] = "/conf/tcp/proto/thread_count";
const char PROTO_TCP_COMMON_LOGIC_SO_XPATH[] = "/conf/tcp/proto/common_logic_so";
const char PROTO_TCP_LOGIC_SO_XPATH[] = "/conf/tcp/proto/logic_so_group/logic_so";
const char WEB_USE_BUFFEREVENT_XPATH[] = "/conf/tcp/web/use_bufferevent";
const char WEB_PART_MSG_CHECK_INTERVAL_XPATH[] = "/conf/tcp/web/part_msg/check_interval";
const char WEB_PART_MSG_CONN_LIFE_XPATH[] = "/conf/tcp/web/part_msg/conn_life";
const char WEB_ADDR_XPATH[] = "/conf/tcp/web/addr";
const char WEB_PORT_XPATH[] = "/conf/tcp/web/port";
const char WEB_SECURITY_PORT_XPATH[] = "/conf/tcp/web/security/port";
const char WEB_CERTIFICATE_CHAIN_FILE_PATH_XPATH[] = "/conf/tcp/web/security/certificate_chain_file_path";
const char WEB_PRIVATE_KEY_FILE_PATH_XPATH[] = "/conf/tcp/web/security/private_key_file_path";
const char WEB_THREAD_COUNT_XPATH[] = "/conf/tcp/web/thread_count";
const char WEB_COMMON_LOGIC_SO_XPATH[] = "/conf/tcp/web/common_logic_so";
const char WEB_LOGIC_SO_XPATH[] = "/conf/tcp/web/logic_so_group/logic_so";
const char UDP_INACTIVE_CONN_CHECK_INTERVAL_SEC_XPATH[] = "/conf/udp/inactive_conn/check_interval/sec";
const char UDP_INACTIVE_CONN_CHECK_INTERVAL_USEC_XPATH[] = "/conf/udp/inactive_conn/check_interval/usec";
const char UDP_INACTIVE_CONN_LIFE_XPATH[] = "/conf/udp/inactive_conn/life";
const char UDP_ADDR_XPATH[] = "/conf/udp/addr";
const char UDP_PORT_XPATH[] = "/conf/udp/port";
const char UDP_THREAD_COUNT_XPATH[] = "/conf/udp/thread_count";
const char UDP_COMMON_LOGIC_SO_XPATH[] = "/conf/udp/common_logic_so";
const char UDP_LOGIC_SO_XPATH[] = "/conf/udp/logic_so_group/logic_so";
const char PEER_RSP_CHECK_INTERVAL_XPATH[] = "/conf/peer/rsp_check_interval";
const char PEER_PROTO_TCP_RECONN_INTERVAL_SEC_XPATH[] = "/conf/peer/tcp/proto/reconn_interval/sec";
const char PEER_PROTO_TCP_RECONN_INTERVAL_USEC_XPATH[] = "/conf/peer/tcp/proto/reconn_interval/usec";
const char PEER_PROTO_TCP_RECONN_LIMIT_XPATH[] = "/conf/peer/tcp/proto/reconn_limit";
const char PEER_HTTP_CONN_TIMEOUT_XPATH[] = "/conf/peer/http/conn_timeout";
const char PEER_HTTP_CONN_MAX_RETRY_XPATH[] = "/conf/peer/http/conn_max_retry";
}

#endif // APP_FRAME_SRC_CONF_XPATH_H_
