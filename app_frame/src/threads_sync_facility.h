#ifndef APP_FRAME_SRC_THREADS_SYNC_FACILITY_H_
#define APP_FRAME_SRC_THREADS_SYNC_FACILITY_H_

#include <pthread.h>

// 启动时主线程与与子线程同步
extern int g_frame_threads_count;
extern pthread_mutex_t g_frame_threads_mutex;
extern pthread_cond_t g_frame_threads_cond;

#endif // APP_FRAME_SRC_THREADS_SYNC_FACILITY_H_
