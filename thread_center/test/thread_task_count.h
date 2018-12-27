#ifndef THREAD_CENTER_TEST_THREAD_TASK_COUNT_H_
#define THREAD_CENTER_TEST_THREAD_TASK_COUNT_H_

#include <atomic>

namespace thread_center_test
{
extern std::atomic<int> g_thread_task_count;
}

#endif // THREAD_CENTER_TEST_THREAD_TASK_COUNT_H_
