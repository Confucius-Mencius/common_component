#ifndef APP_FRAME_SRC_CONF_XPATH_DEFINE_H_
#define APP_FRAME_SRC_CONF_XPATH_DEFINE_H_

namespace app_frame
{
const char ENABLE_CPU_PROFILING_XPATH[] = "/conf/gperftools/enable_cpu_profiling";
const char ENABLE_MEM_PROFILING_XPATH[] = "/conf/gperftools/enable_mem_profiling";
const char RELEASE_FREE_MEM_XPATH[] = "/conf/gperftools/release_free_mem";
const char GLOBAL_COMMON_LOGIC_SO_XPATH[] = "/conf/global/common_logic_so";
const char GLOBAL_LOGIC_SO_XPATH[] = "/conf/global/logic_so_group/logic_so";
const char TCP_ADDR_PORT_XPATH[] = "/conf/tcp/addr_port";
const char TCP_CONN_COUNT_LIMIT_XPATH[] = "/conf/tcp/conn_count_limit";
const char TCP_INACTIVE_CONN_CHECK_INTERVAL_SEC_XPATH[] = "/conf/tcp/inactive_conn/check_interval/sec";
const char TCP_INACTIVE_CONN_CHECK_INTERVAL_USEC_XPATH[] = "/conf/tcp/inactive_conn/check_interval/usec";
const char TCP_INACTIVE_CONN_LIFE_XPATH[] = "/conf/tcp/inactive_conn/life";
const char TCP_STORM_INTERVAL_XPATH[] = "/conf/tcp/storm_interval";
const char TCP_STORM_THRESHOLD_XPATH[] = "/conf/tcp/storm_threshold";
const char TCP_THREAD_COUNT_XPATH[] = "/conf/tcp/thread_count";
const char TCP_COMMON_LOGIC_SO_XPATH[] = "/conf/tcp/common_logic_so";
const char TCP_LOGIC_SO_XPATH[] = "/conf/tcp/logic_so_group/logic_so";
const char UDP_ADDR_PORT_XPATH[] = "/conf/udp/addr_port";
const char UDP_INACTIVE_CONN_CHECK_INTERVAL_SEC_XPATH[] = "/conf/udp/inactive_conn/check_interval/sec";
const char UDP_INACTIVE_CONN_CHECK_INTERVAL_USEC_XPATH[] = "/conf/udp/inactive_conn/check_interval/usec";
const char UDP_INACTIVE_CONN_LIFE_XPATH[] = "/conf/udp/inactive_conn/life";
const char UDP_THREAD_COUNT_XPATH[] = "/conf/udp/thread_count";
const char UDP_COMMON_LOGIC_SO_XPATH[] = "/conf/udp/common_logic_so";
const char UDP_LOGIC_SO_XPATH[] = "/conf/udp/logic_so_group/logic_so";
const char WORK_THREAD_COUNT_XPATH[] = "/conf/work/thread_count";
const char WORK_COMMON_LOGIC_SO_XPATH[] = "/conf/work/common_logic_so";
const char WORK_LOGIC_SO_XPATH[] = "/conf/work/logic_so_group/logic_so";
const char IO_TO_WORK_TQ_SIZE_LIMIT_XPATH[] = "/conf/io_to_work_tq_size_limit";
const char BURDEN_THREAD_COUNT_XPATH[] = "/conf/burden/thread_count";
const char BURDEN_COMMON_LOGIC_SO_XPATH[] = "/conf/burden/common_logic_so";
const char BURDEN_LOGIC_SO_XPATH[] = "/conf/burden/logic_so_group/logic_so";
}

#endif // APP_FRAME_SRC_CONF_XPATH_DEFINE_H_
