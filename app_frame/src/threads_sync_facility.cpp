#include "threads_sync_facility.h"

int g_frame_threads_count = 0;
pthread_mutex_t g_frame_threads_mutex;
pthread_cond_t g_frame_threads_cond;
