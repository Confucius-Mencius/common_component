#ifndef HTTP_THREADS_SRC_HTTP_THREADS_SYNC_FACILITY_H_
#define HTTP_THREADS_SRC_HTTP_THREADS_SYNC_FACILITY_H_

#include <pthread.h>

// 对http/https同时在线的用户数进行限制（所有线程）
extern int g_online_http_conn_count;
extern int g_max_online_http_conn_count;
extern pthread_mutex_t g_online_http_conn_count_mutex;

#endif // HTTP_THREADS_SRC_HTTP_THREADS_SYNC_FACILITY_H_
