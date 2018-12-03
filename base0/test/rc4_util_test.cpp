#include "rc4_util_test.h"
#include <sys/time.h>
#include "rc4_util.h"
#include "hex_dump.h"
#include "log4cplus/logger.h"
#include "log4cplus/consoleappender.h"
#include "log4cplus/loggingmacros.h"

RC4UtilTest::RC4UtilTest()
{

}

RC4UtilTest::~RC4UtilTest()
{

}

void RC4UtilTest::Test001()
{
    unsigned char key_data[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef}; // sizeof(key_data) = 8

    // 原始数据
    const unsigned char in_data[] = "this is a rc4 test case"; // sizeof(in_data) = 24
    int in_data_len = strlen((const char*) in_data); // in_data_len = 23

    char hex_dump_buf[4096] = "";

    memset(hex_dump_buf, 0, sizeof(hex_dump_buf));
    HexDump(hex_dump_buf, sizeof(hex_dump_buf), in_data, in_data_len);
    std::cout << hex_dump_buf << std::endl;

    // 加密
    unsigned char* out_data = NULL;
    if (RC4Encode(&out_data, in_data, in_data_len, key_data, sizeof(key_data)) != 0)
    {
        RC4Release(&out_data);
        FAIL();
    }

    //  取出结果
    unsigned char* out_data_buf = new unsigned char[in_data_len + 1];
    out_data_buf[in_data_len] = '\0';
    memset(out_data_buf, 0, in_data_len);
    memcpy(out_data_buf, out_data, in_data_len);

    // 释放内存
    RC4Release(&out_data);

    memset(hex_dump_buf, 0, sizeof(hex_dump_buf));
    HexDump(hex_dump_buf, sizeof(hex_dump_buf), out_data_buf, in_data_len);
    std::cout << hex_dump_buf << std::endl;

    // 解密
    unsigned char* origin_in_data = NULL;
    if (RC4Decode(&origin_in_data, out_data_buf, in_data_len, key_data, sizeof(key_data)) != 0)
    {
        RC4Release(&origin_in_data);

        delete[] out_data_buf;
        out_data_buf = NULL;

        FAIL();
    }

    std::string s((const char*) origin_in_data, in_data_len);
    std::cout << "s: " << s << ", s.size(): " << s.size() << std::endl;
    EXPECT_STREQ((const char*) in_data, s.c_str());

    memset(hex_dump_buf, 0, sizeof(hex_dump_buf));
    HexDump(hex_dump_buf, sizeof(hex_dump_buf), origin_in_data, in_data_len);
    std::cout << hex_dump_buf << std::endl;

    RC4Release(&origin_in_data);
}

namespace rc4_util_test
{
log4cplus::Logger g_testLogger;

void* ThreadProc(void* ctx)
{
    int* i = (int*) ctx;

    pthread_t tid = pthread_self();

    unsigned char key_data[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef}; // sizeof(key_data) = 8

    // 原始数据
    const int in_data_len = 1024 * 1024;
    unsigned char* in_data = new unsigned char[in_data_len + 1]; // 1M bytes
    for (int i = 0; i < in_data_len; ++i)
    {
        in_data[i] = rand() % 256;
    }

    in_data[in_data_len] = '\0';

    struct timeval start;
    struct timeval end;

    gettimeofday(&start, NULL);

    for (int i = 0; i < 10; ++i)
    {
        // 加密
        unsigned char* out_data = NULL;
        if (RC4Encode(&out_data, in_data, in_data_len, key_data, sizeof(key_data)) != 0)
        {
            RC4Release(&out_data);
            return (void*) -1;
        }

        RC4Release(&out_data);
    }

    gettimeofday(&end, NULL);

    LOG4CPLUS_INFO(rc4_util_test::g_testLogger,
                   *i << ", tid: " << tid << " start: " << start.tv_sec << ", " << start.tv_usec << " end: " <<
                       end.tv_sec << ", " << end.tv_usec << " time sec: " << end.tv_sec - start.tv_sec <<
                       " time usec: " << end.tv_usec - start.tv_usec);

    return (void*) 0;
}
}

// 计算md5+rc4时间测试
void RC4UtilTest::Test002()
{
    // 定义一个控制台的Appender
    log4cplus::SharedAppenderPtr pConsoleAppender(new log4cplus::ConsoleAppender());

    // 定义Logger
    rc4_util_test::g_testLogger = log4cplus::Logger::getInstance("LoggerName");

    // 将需要关联Logger的Appender添加到Logger上
    rc4_util_test::g_testLogger.addAppender(pConsoleAppender);

    // 输出日志信息
    LOG4CPLUS_WARN(rc4_util_test::g_testLogger, "This is a <Warn> log message...");

//    pthread_attr_t attr;
//    pthread_attr_init(&attr);
//    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

    pthread_t tid[32];
    int idx[32];

    for (int i = 0; i < (int) (sizeof(tid) / sizeof(tid[0])); ++i)
    {
        idx[i] = i;
        pthread_create(&tid[i], NULL, rc4_util_test::ThreadProc, &idx[i]);
        //pthread_create(&tid[i], &attr, rc4_util_test::ThreadProc, &idx[i]);
    }

    //pthread_attr_destroy(&attr);

    for (int i = 0; i < (int) (sizeof(tid) / sizeof(tid[0])); ++i)
    {
        pthread_join(tid[i], NULL);
    }
}

ADD_TEST_F(RC4UtilTest, Test001);
ADD_TEST_F(RC4UtilTest, Test002);
