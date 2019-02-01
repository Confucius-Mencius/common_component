#ifndef APP_FRAME_SRC_THREADS_SYNC_FACILITY_H_
#define APP_FRAME_SRC_THREADS_SYNC_FACILITY_H_

#include <pthread.h>

// 启动时主线程与与子线程同步
namespace app_frame
{
int g_threads_count = 0;
pthread_mutex_t g_threads_sync_mutex;
pthread_cond_t g_threads_sync_cond;
}

#endif // APP_FRAME_SRC_THREADS_SYNC_FACILITY_H_
