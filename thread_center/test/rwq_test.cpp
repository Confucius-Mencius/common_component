#include "rwq_test.h"
#include <thread>
#include "readerwriterqueue.h"

namespace thread_center_test
{
RWQTest::RWQTest()
{
}

RWQTest::~RWQTest()
{
}

void RWQTest::Test001()
{
    using namespace moodycamel;

    ReaderWriterQueue<int> q(100);       // Reserve space for at least 100 elements up front

    q.enqueue(17);                       // Will allocate memory if the queue is full
    bool succeeded = q.try_enqueue(18);  // Will only succeed if the queue has an empty slot (never allocates)
    assert(succeeded);

    int number;
    succeeded = q.try_dequeue(number);  // Returns false if the queue was empty

    assert(succeeded && number == 17);

    // You can also peek at the front item of the queue (consumer only)
    int* front = q.peek();
    assert(*front == 18);
    succeeded = q.try_dequeue(number);
    assert(succeeded && number == 18);
    front = q.peek();
    assert(front == nullptr);           // Returns nullptr if the queue was empty
}

void RWQTest::Test002()
{
    // The blocking version has the exact same API, with the addition of wait_dequeue and wait_dequeue_timed methods
    // Note that wait_dequeue will block indefinitely while the queue is empty
    using namespace moodycamel;

    BlockingReaderWriterQueue<int> q;

    std::thread reader([&]()
    {
        int item;
        for (int i = 0; i != 100; ++i)
        {
            // Fully-blocking:
            q.wait_dequeue(item);

            // Blocking with timeout
            if (q.wait_dequeue_timed(item, std::chrono::milliseconds(5)))
            {
                ++i;
            }
        }
    });
    std::thread writer([&]()
    {
        for (int i = 0; i != 100; ++i)
        {
            q.enqueue(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    writer.join();
    reader.join();

    assert(q.size_approx() == 0);
}

ADD_TEST_F(RWQTest, Test001);
ADD_TEST_F(RWQTest, Test002);
}
