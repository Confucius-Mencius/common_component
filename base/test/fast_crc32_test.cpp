#include "fast_crc32_test.h"
#include "fast_crc32.h"
#include "simple_log.h"

namespace fast_crc32_test
{
FastCRC32Test::FastCRC32Test()
{
}

FastCRC32Test::~FastCRC32Test()
{
}

void FastCRC32Test::Test001()
{
    const char data[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const int data_len = sizeof(data) - 1;

    uint32_t data_crc32 = FastCRC32(data, data_len);
    LOG_CPP(data_crc32);
}

void FastCRC32Test::Test002()
{
    uint32_t file_crc32 = FileFastCRC32("./data/fast_crc32_file");
    ASSERT_TRUE(file_crc32 != 0);
    LOG_CPP(file_crc32);
}

ADD_TEST_F(FastCRC32Test, Test001);
ADD_TEST_F(FastCRC32Test, Test002);
}
