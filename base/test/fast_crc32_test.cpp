#include "fast_crc32_test.h"
#include "fast_crc32.h"

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
    std::cout << data_crc32 << std::endl;
}

void FastCRC32Test::Test002()
{
    uint32_t file_crc32 = FastFileCRC32("./myfile");
    std::cout << file_crc32 << std::endl;
}

ADD_TEST_F(FastCRC32Test, Test001);
ADD_TEST_F(FastCRC32Test, Test002);
