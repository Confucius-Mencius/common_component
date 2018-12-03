#include "http_threads_sync_facility.h"

int g_online_http_conn_count = 0;
int g_max_online_http_conn_count = 0;
pthread_mutex_t g_online_http_conn_count_mutex;
