#include "zip_test.h"
#include <fcntl.h>
#include "zip_util.h"
#include "fast_crc32.h"

ZipTest::ZipTest()
{

}

ZipTest::~ZipTest()
{

}

void ZipTest::Test001()
{
    FILE* fp = fopen("./零零一.zip", "w");

    uint16_t central_directory_count = 0;
    uint32_t total_central_directory_size = 0;

    uint64_t local_file_header_offset_1 = 0;

    // file1
    char file_name_1[] = "一/二/阿.txt";
    char buf_1[] = "123456";
    uint32_t file_size_1 = sizeof(buf_1) - 1;

    uint32_t local_file_header_len_1 = GetLocalFileHeaderLen(file_name_1, file_size_1);
    char* local_file_header_1 = (char*) malloc(local_file_header_len_1);
    BuildLocalFileHeader(local_file_header_1, local_file_header_len_1, file_name_1, file_size_1, 158520161);

    fwrite(local_file_header_1, local_file_header_len_1, 1, fp);
    free(local_file_header_1);

    // file1 data
    fwrite(buf_1, file_size_1, 1, fp);

    uint64_t local_file_header_offset_2 = local_file_header_len_1 + file_size_1;

    // file2
    char file_name_2[] = "一/二/波.txt";
    char buf_2[] = "789";
    uint32_t file_size_2 = sizeof(buf_2) - 1;

    uint32_t local_file_header_len_2 = GetLocalFileHeaderLen(file_name_2, file_size_2);
    char* local_file_header_2 = (char*) malloc(local_file_header_len_2);
    BuildLocalFileHeader(local_file_header_2, local_file_header_len_2, file_name_2, file_size_2, 0x96ff1ef4);

    fwrite(local_file_header_2, local_file_header_len_2, 1, fp);
    free(local_file_header_2);

    // file2 data
    fwrite(buf_2, file_size_2, 1, fp);

    uint64_t local_file_header_offset_3 = local_file_header_offset_2 + local_file_header_len_2 + file_size_2;

    // dir1
    char file_name_3[] = "天/地/";
    uint32_t file_size_3 = 0;

    uint32_t local_file_header_len_3 = GetLocalFileHeaderLen(file_name_3, file_size_3);
    char* local_file_header_3 = (char*) malloc(local_file_header_len_3);
    BuildLocalFileHeader(local_file_header_3, local_file_header_len_3, file_name_3, 0, 0);

    fwrite(local_file_header_3, local_file_header_len_3, 1, fp);
    free(local_file_header_3);

    uint64_t first_central_directory_offset = local_file_header_offset_3 + local_file_header_len_3 + file_size_3;

    ////////////////////////////////////////////////////////////////////////////////
    // central directory

    // file1
    uint32_t central_directory_len_1 = GetCentralDirectoryLen(file_name_1, file_size_1, local_file_header_offset_1);
    char* central_directory_1 = (char*) malloc(central_directory_len_1);
    BuildCentralDirectory(central_directory_1, central_directory_len_1, file_name_1, file_size_1,
                          local_file_header_offset_1, 158520161);

    fwrite(central_directory_1, central_directory_len_1, 1, fp);
    free(central_directory_1);

    ++central_directory_count;
    total_central_directory_size += central_directory_len_1;

    // file2
    uint32_t central_directory_len_2 = GetCentralDirectoryLen(file_name_2, file_size_2, local_file_header_offset_2);
    char* central_directory_2 = (char*) malloc(central_directory_len_2);
    BuildCentralDirectory(central_directory_2, central_directory_len_2, file_name_2, file_size_2,
                          local_file_header_offset_2, 0x96ff1ef4);

    fwrite(central_directory_2, central_directory_len_2, 1, fp);
    free(central_directory_2);

    ++central_directory_count;
    total_central_directory_size += central_directory_len_2;

    // dir1
    uint32_t central_directory_len_3 = GetCentralDirectoryLen(file_name_3, file_size_3, local_file_header_offset_3);
    char* central_directory_3 = (char*) malloc(central_directory_len_3);
    BuildCentralDirectory(central_directory_3, central_directory_len_3, file_name_3, file_size_3,
                          local_file_header_offset_3, 0);

    fwrite(central_directory_3, central_directory_len_3, 1, fp);
    free(central_directory_3);

    ++central_directory_count;
    total_central_directory_size += central_directory_len_3;

    uint64_t zip64_central_directory_end_offset = first_central_directory_offset + total_central_directory_size;

    // end
    uint32_t zip64_central_directory_end_len = GetZip64CentralDirectoryEndLen();
    char* zip64_central_directory_end = (char*) malloc(zip64_central_directory_end_len);
    BuildZip64CentralDirectoryEnd(zip64_central_directory_end, zip64_central_directory_end_len, central_directory_count,
                                  total_central_directory_size, first_central_directory_offset);

    fwrite(zip64_central_directory_end, zip64_central_directory_end_len, 1, fp);
    free(zip64_central_directory_end);

    uint32_t zip64_central_directory_locator_len = GetZip64CentralDirectoryLocatorLen();
    char* zip64_central_directory_locator = (char*) malloc(zip64_central_directory_locator_len);
    BuildZip64CentralDirectoryLocator(zip64_central_directory_locator, zip64_central_directory_locator_len,
                                      zip64_central_directory_end_offset);

    fwrite(zip64_central_directory_locator, zip64_central_directory_locator_len, 1, fp);
    free(zip64_central_directory_locator);

    uint32_t central_directory_end_len = GetCentralDirectoryEndLen();
    char* central_directory_end = (char*) malloc(central_directory_end_len);
    BuildCentralDirectoryEnd(central_directory_end, central_directory_end_len, central_directory_count,
                             total_central_directory_size, first_central_directory_offset);

    fwrite(central_directory_end, central_directory_end_len, 1, fp);
    free(central_directory_end);

    fclose(fp);
}

void ZipTest::Test002()
{
    FILE* fp = fopen("./002.zip", "w");

    uint16_t central_directory_count = 0;
    uint32_t total_central_directory_size = 0;

    uint64_t local_file_header_offset_1 = 0;

    // file1
    char file_name_1[] = "1/2/a.txt";
    char buf_1[] = "123456";
    uint32_t file_size_1 = sizeof(buf_1) - 1;

    uint32_t local_file_header_len_1 = GetLocalFileHeaderLen(file_name_1, file_size_1);
    char* local_file_header_1 = (char*) malloc(local_file_header_len_1);
    BuildLocalFileHeader(local_file_header_1, local_file_header_len_1, file_name_1, file_size_1, 158520161);

    fwrite(local_file_header_1, local_file_header_len_1, 1, fp);
    free(local_file_header_1);

    // file1 data
    fwrite(buf_1, file_size_1, 1, fp);

    uint64_t local_file_header_offset_2 = local_file_header_len_1 + file_size_1;

    // file2
    char file_name_2[] = "1/2/b.txt";
    char buf_2[] = "789";
    uint32_t file_size_2 = sizeof(buf_2) - 1;

    uint32_t local_file_header_len_2 = GetLocalFileHeaderLen(file_name_2, file_size_2);
    char* local_file_header_2 = (char*) malloc(local_file_header_len_2);
    BuildLocalFileHeader(local_file_header_2, local_file_header_len_2, file_name_2, file_size_2, 0x96ff1ef4);

    fwrite(local_file_header_2, local_file_header_len_2, 1, fp);
    free(local_file_header_2);

    // file2 data
    fwrite(buf_2, file_size_2, 1, fp);

    uint64_t local_file_header_offset_3 = local_file_header_offset_2 + local_file_header_len_2 + file_size_2;

    // dir1
    char file_name_3[] = "1/2/3";
    uint32_t file_size_3 = 0;

    uint32_t local_file_header_len_3 = GetLocalFileHeaderLen(file_name_3, file_size_3);
    char* local_file_header_3 = (char*) malloc(local_file_header_len_3);
    BuildLocalFileHeader(local_file_header_3, local_file_header_len_3, file_name_3, 0, 0);

    fwrite(local_file_header_3, local_file_header_len_3, 1, fp);
    free(local_file_header_3);

    uint64_t first_central_directory_offset = local_file_header_offset_3 + local_file_header_len_3 + file_size_3;

    ////////////////////////////////////////////////////////////////////////////////
    // central directory

    // file1
    uint32_t central_directory_len_1 = GetCentralDirectoryLen(file_name_1, file_size_1, local_file_header_offset_1);
    char* central_directory_1 = (char*) malloc(central_directory_len_1);
    BuildCentralDirectory(central_directory_1, central_directory_len_1, file_name_1, file_size_1,
                          local_file_header_offset_1, 158520161);

    fwrite(central_directory_1, central_directory_len_1, 1, fp);
    free(central_directory_1);

    ++central_directory_count;
    total_central_directory_size += central_directory_len_1;

    // file2
    uint32_t central_directory_len_2 = GetCentralDirectoryLen(file_name_2, file_size_2, local_file_header_offset_2);
    char* central_directory_2 = (char*) malloc(central_directory_len_2);
    BuildCentralDirectory(central_directory_2, central_directory_len_2, file_name_2, file_size_2,
                          local_file_header_offset_2, 0x96ff1ef4);

    fwrite(central_directory_2, central_directory_len_2, 1, fp);
    free(central_directory_2);

    ++central_directory_count;
    total_central_directory_size += central_directory_len_2;

    // dir1
    uint32_t central_directory_len_3 = GetCentralDirectoryLen(file_name_3, file_size_3, local_file_header_offset_3);
    char* central_directory_3 = (char*) malloc(central_directory_len_3);
    BuildCentralDirectory(central_directory_3, central_directory_len_3, file_name_3, file_size_3,
                          local_file_header_offset_3, 0);

    fwrite(central_directory_3, central_directory_len_3, 1, fp);
    free(central_directory_3);

    ++central_directory_count;
    total_central_directory_size += central_directory_len_3;

    uint64_t zip64_central_directory_end_offset = first_central_directory_offset + total_central_directory_size;

    // end
    uint32_t zip64_central_directory_end_len = GetZip64CentralDirectoryEndLen();
    char* zip64_central_directory_end = (char*) malloc(zip64_central_directory_end_len);
    BuildZip64CentralDirectoryEnd(zip64_central_directory_end, zip64_central_directory_end_len, central_directory_count,
                                  total_central_directory_size, first_central_directory_offset);

    fwrite(zip64_central_directory_end, zip64_central_directory_end_len, 1, fp);
    free(zip64_central_directory_end);

    uint32_t zip64_central_directory_locator_len = GetZip64CentralDirectoryLocatorLen();
    char* zip64_central_directory_locator = (char*) malloc(zip64_central_directory_locator_len);
    BuildZip64CentralDirectoryLocator(zip64_central_directory_locator, zip64_central_directory_locator_len,
                                      zip64_central_directory_end_offset);

    fwrite(zip64_central_directory_locator, zip64_central_directory_locator_len, 1, fp);
    free(zip64_central_directory_locator);

    uint32_t central_directory_end_len = GetCentralDirectoryEndLen();
    char* central_directory_end = (char*) malloc(central_directory_end_len);
    BuildCentralDirectoryEnd(central_directory_end, central_directory_end_len, central_directory_count,
                             total_central_directory_size, first_central_directory_offset);

    fwrite(central_directory_end, central_directory_end_len, 1, fp);
    free(central_directory_end);

    fclose(fp);
}

void ZipTest::Test003()
{
    char src_file_name_1[] = "./1";
    char src_file_name_2[] = "./2";

    // src file 1
    int src_file_fd_1 = open(src_file_name_1, O_RDONLY);
    if (src_file_fd_1 < 0)
    {
        FAIL();
    }

    struct stat src_file_stat_1;
    fstat(src_file_fd_1, &src_file_stat_1);
    uint64_t src_file_size_1 = src_file_stat_1.st_size;
    ASSERT_EQ(UINT64_C(1044381696), src_file_size_1);

    uint32_t src_file_crc32_1 = 0;

    uint32_t buf_size = 10 * 1024 * 1024; // 一次读10M
    char* buf = (char*) malloc(buf_size);
    uint64_t size_left = src_file_size_1;

    while (size_left > 0)
    {
        uint32_t n = read(src_file_fd_1, buf, buf_size);
        src_file_crc32_1 = FastCRC32(buf, n, src_file_crc32_1);

        if (size_left > buf_size)
        {
            size_left -= buf_size;
        }
        else
        {
            size_left = 0;
        }
    }

    ASSERT_EQ((uint32_t) 0x34A249A2, src_file_crc32_1);
    close(src_file_fd_1);

    // src file 2
    int src_file_fd_2 = open(src_file_name_2, O_RDONLY);
    if (src_file_fd_2 < 0)
    {
        FAIL();
    }

    struct stat src_file_stat_2;
    fstat(src_file_fd_2, &src_file_stat_2);
    uint64_t src_file_size_2 = src_file_stat_2.st_size;
    ASSERT_EQ(UINT64_C(4148166656), src_file_size_2);

    uint32_t src_file_crc32_2 = 0;

    size_left = src_file_size_2;

    while (size_left > 0)
    {
        uint32_t n = read(src_file_fd_2, buf, buf_size);
        src_file_crc32_2 = FastCRC32(buf, n, src_file_crc32_2);

        if (size_left > buf_size)
        {
            size_left -= buf_size;
        }
        else
        {
            size_left = 0;
        }
    }

    ASSERT_EQ((uint32_t) 0x5F2AC2AD, src_file_crc32_2);
    close(src_file_fd_2);

    // target zip file
    int target_file_fd = open("003.zip", O_WRONLY | O_CREAT | O_TRUNC, 00666);
    if (target_file_fd < 0)
    {
        FAIL();
    }

    uint16_t central_directory_count = 0;
    uint32_t total_central_directory_size = 0;

    uint64_t local_file_header_offset_1 = 0;

    // file1
    char file_name_1[] = "a/b/1";

    uint32_t local_file_header_len_1 = GetLocalFileHeaderLen(file_name_1, src_file_size_1);
    char* local_file_header_1 = (char*) malloc(local_file_header_len_1);
    BuildLocalFileHeader(local_file_header_1, local_file_header_len_1, file_name_1, src_file_size_1, src_file_crc32_1);
    write(target_file_fd, local_file_header_1, local_file_header_len_1);
    free(local_file_header_1);

    // file1 data
    src_file_fd_1 = open(src_file_name_1, O_RDONLY);
    size_left = src_file_size_1;

    while (size_left > 0)
    {
        uint32_t n = read(src_file_fd_1, buf, buf_size);
        write(target_file_fd, buf, n);

        if (size_left > buf_size)
        {
            size_left -= buf_size;
        }
        else
        {
            size_left = 0;
        }
    }

    close(src_file_fd_1);

    uint64_t local_file_header_offset_2 = local_file_header_len_1 + src_file_size_1;

    // file2
    char file_name_2[] = "a/b/2";

    uint32_t local_file_header_len_2 = GetLocalFileHeaderLen(file_name_2, src_file_size_2);
    char* local_file_header_2 = (char*) malloc(local_file_header_len_2);
    BuildLocalFileHeader(local_file_header_2, local_file_header_len_2, file_name_2, src_file_size_2, src_file_crc32_2);
    write(target_file_fd, local_file_header_2, local_file_header_len_2);
    free(local_file_header_2);

    // file2 data
    src_file_fd_2 = open(src_file_name_2, O_RDONLY);
    size_left = src_file_size_2;

    while (size_left > 0)
    {
        uint32_t n = read(src_file_fd_2, buf, buf_size);
        write(target_file_fd, buf, n);

        if (size_left > buf_size)
        {
            size_left -= buf_size;
        }
        else
        {
            size_left = 0;
        }
    }

    close(src_file_fd_2);

    uint64_t local_file_header_offset_3 = local_file_header_offset_2 + local_file_header_len_2 + src_file_size_2;

    // file3, empty file
    char file_name_3[] = "a/b/3";
    uint32_t file_size_3 = 0;

    uint32_t local_file_header_len_3 = GetLocalFileHeaderLen(file_name_3, file_size_3);
    char* local_file_header_3 = (char*) malloc(local_file_header_len_3);
    BuildLocalFileHeader(local_file_header_3, local_file_header_len_3, file_name_3, 0, 0);
    write(target_file_fd, local_file_header_3, local_file_header_len_3);
    free(local_file_header_3);

    uint64_t first_central_directory_offset = local_file_header_offset_3 + local_file_header_len_3 + file_size_3;

    ////////////////////////////////////////////////////////////////////////////////
    // central directory

    // file1
    uint32_t central_directory_len_1 = GetCentralDirectoryLen(file_name_1, src_file_size_1, local_file_header_offset_1);
    char* central_directory_1 = (char*) malloc(central_directory_len_1);
    BuildCentralDirectory(central_directory_1, central_directory_len_1, file_name_1, src_file_size_1,
                          local_file_header_offset_1, src_file_crc32_1);

    write(target_file_fd, central_directory_1, central_directory_len_1);
    free(central_directory_1);

    ++central_directory_count;
    total_central_directory_size += central_directory_len_1;

    // file2
    uint32_t central_directory_len_2 = GetCentralDirectoryLen(file_name_2, src_file_size_2, local_file_header_offset_2);
    char* central_directory_2 = (char*) malloc(central_directory_len_2);
    BuildCentralDirectory(central_directory_2, central_directory_len_2, file_name_2, src_file_size_2,
                          local_file_header_offset_2, src_file_crc32_2);

    write(target_file_fd, central_directory_2, central_directory_len_2);
    free(central_directory_2);

    ++central_directory_count;
    total_central_directory_size += central_directory_len_2;

    // file3
    uint32_t central_directory_len_3 = GetCentralDirectoryLen(file_name_3, file_size_3, local_file_header_offset_3);
    char* central_directory_3 = (char*) malloc(central_directory_len_3);
    BuildCentralDirectory(central_directory_3, central_directory_len_3, file_name_3, file_size_3,
                          local_file_header_offset_3, 0);

    write(target_file_fd, central_directory_3, central_directory_len_3);
    free(central_directory_3);

    ++central_directory_count;
    total_central_directory_size += central_directory_len_3;

    uint64_t zip64_central_directory_end_offset = first_central_directory_offset + total_central_directory_size;

    // end
    uint32_t zip64_central_directory_end_len = GetZip64CentralDirectoryEndLen();
    char* zip64_central_directory_end = (char*) malloc(zip64_central_directory_end_len);
    BuildZip64CentralDirectoryEnd(zip64_central_directory_end, zip64_central_directory_end_len, central_directory_count,
                                  total_central_directory_size, first_central_directory_offset);

    write(target_file_fd, zip64_central_directory_end, zip64_central_directory_end_len);
    free(zip64_central_directory_end);

    uint32_t zip64_central_directory_locator_len = GetZip64CentralDirectoryLocatorLen();
    char* zip64_central_directory_locator = (char*) malloc(zip64_central_directory_locator_len);
    BuildZip64CentralDirectoryLocator(zip64_central_directory_locator, zip64_central_directory_locator_len,
                                      zip64_central_directory_end_offset);

    write(target_file_fd, zip64_central_directory_locator, zip64_central_directory_locator_len);
    free(zip64_central_directory_locator);

    uint32_t central_directory_end_len = GetCentralDirectoryEndLen();
    char* central_directory_end = (char*) malloc(central_directory_end_len);
    BuildCentralDirectoryEnd(central_directory_end, central_directory_end_len, central_directory_count,
                             total_central_directory_size, first_central_directory_offset);

    write(target_file_fd, central_directory_end, central_directory_end_len);
    free(central_directory_end);

    close(target_file_fd);

    free(buf);
}

void ZipTest::Test004()
{
    char src_file_name_1[] = "./vb.zip";

    // src file 1
    int src_file_fd_1 = open(src_file_name_1, O_RDONLY);
    if (src_file_fd_1 < 0)
    {
        FAIL();
    }

    struct stat src_file_stat_1;
    fstat(src_file_fd_1, &src_file_stat_1);
    uint64_t src_file_size_1 = src_file_stat_1.st_size;

    uint32_t src_file_crc32_1 = 0;

    uint32_t buf_size = 10 * 1024 * 1024; // 一次读10M
    char* buf = (char*) malloc(buf_size);
    uint64_t size_left = src_file_size_1;

    while (size_left > 0)
    {
        uint32_t n = read(src_file_fd_1, buf, buf_size);
        src_file_crc32_1 = FastCRC32(buf, n, src_file_crc32_1);

        if (size_left > buf_size)
        {
            size_left -= buf_size;
        }
        else
        {
            size_left = 0;
        }
    }

    close(src_file_fd_1);

    // target zip file
    int target_file_fd = open("004.zip", O_WRONLY | O_CREAT | O_TRUNC, 00666);
    if (target_file_fd < 0)
    {
        FAIL();
    }

    uint16_t central_directory_count = 0;
    uint32_t total_central_directory_size = 0;

    uint64_t local_file_header_offset_1 = 0;

    // file1
    char file_name_1[] = "a/b/1";

    uint32_t local_file_header_len_1 = GetLocalFileHeaderLen(file_name_1, src_file_size_1);
    char* local_file_header_1 = (char*) malloc(local_file_header_len_1);
    BuildLocalFileHeader(local_file_header_1, local_file_header_len_1, file_name_1, src_file_size_1, src_file_crc32_1);
    write(target_file_fd, local_file_header_1, local_file_header_len_1);
    free(local_file_header_1);

    // file1 data
    src_file_fd_1 = open(src_file_name_1, O_RDONLY);
    size_left = src_file_size_1;

    while (size_left > 0)
    {
        uint32_t n = read(src_file_fd_1, buf, buf_size);
        write(target_file_fd, buf, n);

        if (size_left > buf_size)
        {
            size_left -= buf_size;
        }
        else
        {
            size_left = 0;
        }
    }

    close(src_file_fd_1);

    uint64_t first_central_directory_offset = local_file_header_offset_1 + local_file_header_len_1 + src_file_size_1;

    ////////////////////////////////////////////////////////////////////////////////
    // central directory

    // file1
    uint32_t central_directory_len_1 = GetCentralDirectoryLen(file_name_1, src_file_size_1, local_file_header_offset_1);
    char* central_directory_1 = (char*) malloc(central_directory_len_1);
    BuildCentralDirectory(central_directory_1, central_directory_len_1, file_name_1, src_file_size_1,
                          local_file_header_offset_1, src_file_crc32_1);

    write(target_file_fd, central_directory_1, central_directory_len_1);
    free(central_directory_1);

    ++central_directory_count;
    total_central_directory_size += central_directory_len_1;

    uint64_t zip64_central_directory_end_offset = first_central_directory_offset + total_central_directory_size;

    // end
    uint32_t zip64_central_directory_end_len = GetZip64CentralDirectoryEndLen();
    char* zip64_central_directory_end = (char*) malloc(zip64_central_directory_end_len);
    BuildZip64CentralDirectoryEnd(zip64_central_directory_end, zip64_central_directory_end_len, central_directory_count,
                                  total_central_directory_size, first_central_directory_offset);

    write(target_file_fd, zip64_central_directory_end, zip64_central_directory_end_len);
    free(zip64_central_directory_end);

    uint32_t zip64_central_directory_locator_len = GetZip64CentralDirectoryLocatorLen();
    char* zip64_central_directory_locator = (char*) malloc(zip64_central_directory_locator_len);
    BuildZip64CentralDirectoryLocator(zip64_central_directory_locator, zip64_central_directory_locator_len,
                                      zip64_central_directory_end_offset);

    write(target_file_fd, zip64_central_directory_locator, zip64_central_directory_locator_len);
    free(zip64_central_directory_locator);

    uint32_t central_directory_end_len = GetCentralDirectoryEndLen();
    char* central_directory_end = (char*) malloc(central_directory_end_len);
    BuildCentralDirectoryEnd(central_directory_end, central_directory_end_len, central_directory_count,
                             total_central_directory_size, first_central_directory_offset);

    write(target_file_fd, central_directory_end, central_directory_end_len);
    free(central_directory_end);

    close(target_file_fd);

    free(buf);
}

void ZipTest::Test005()
{
    char src_file_name_1[] = "./vb.zip";

    // src file 1
    int src_file_fd_1 = open(src_file_name_1, O_RDONLY);
    if (src_file_fd_1 < 0)
    {
        FAIL();
    }

    struct stat src_file_stat_1;
    fstat(src_file_fd_1, &src_file_stat_1);
    uint64_t src_file_size_1 = src_file_stat_1.st_size;

    uint32_t src_file_crc32_1 = 0;

    uint32_t buf_size = 10 * 1024 * 1024; // 一次读10M
    char* buf = (char*) malloc(buf_size);
    uint64_t size_left = src_file_size_1;

    while (size_left > 0)
    {
        uint32_t n = read(src_file_fd_1, buf, buf_size);
        src_file_crc32_1 = FastCRC32(buf, n, src_file_crc32_1);

        if (size_left > buf_size)
        {
            size_left -= buf_size;
        }
        else
        {
            size_left = 0;
        }
    }

    close(src_file_fd_1);

    // target zip file
    int target_file_fd = open("005.zip", O_WRONLY | O_CREAT | O_TRUNC, 00666);
    if (target_file_fd < 0)
    {
        FAIL();
    }

    uint16_t central_directory_count = 0;
    uint32_t total_central_directory_size = 0;

    uint64_t local_file_header_offset_1 = 0;

    // file1
    char file_name_1[] = "a/b/1";

    uint32_t local_file_header_len_1 = GetLocalFileHeaderLen(file_name_1, src_file_size_1);
    char* local_file_header_1 = (char*) malloc(local_file_header_len_1);
    BuildLocalFileHeader(local_file_header_1, local_file_header_len_1, file_name_1, src_file_size_1, src_file_crc32_1);
    write(target_file_fd, local_file_header_1, local_file_header_len_1);
    free(local_file_header_1);

    // file1 data
    src_file_fd_1 = open(src_file_name_1, O_RDONLY);
    size_left = src_file_size_1;

    while (size_left > 0)
    {
        uint32_t n = read(src_file_fd_1, buf, buf_size);
        write(target_file_fd, buf, n);

        if (size_left > buf_size)
        {
            size_left -= buf_size;
        }
        else
        {
            size_left = 0;
        }
    }

    close(src_file_fd_1);

    uint64_t local_file_header_offset_2 = local_file_header_offset_1 + local_file_header_len_1 + src_file_size_1;

    // file2
    char file_name_2[] = "a/b/2";

    uint32_t local_file_header_len_2 = GetLocalFileHeaderLen(file_name_2, src_file_size_1);
    char* local_file_header_2 = (char*) malloc(local_file_header_len_2);
    BuildLocalFileHeader(local_file_header_2, local_file_header_len_2, file_name_2, src_file_size_1, src_file_crc32_1);
    write(target_file_fd, local_file_header_2, local_file_header_len_2);
    free(local_file_header_2);

    // file1 data
    src_file_fd_1 = open(src_file_name_1, O_RDONLY);
    size_left = src_file_size_1;

    while (size_left > 0)
    {
        uint32_t n = read(src_file_fd_1, buf, buf_size);
        write(target_file_fd, buf, n);

        if (size_left > buf_size)
        {
            size_left -= buf_size;
        }
        else
        {
            size_left = 0;
        }
    }

    close(src_file_fd_1);

    uint64_t first_central_directory_offset = local_file_header_offset_2 + local_file_header_len_2 + src_file_size_1;

    ////////////////////////////////////////////////////////////////////////////////
    // central directory

    // file1
    uint32_t central_directory_len_1 = GetCentralDirectoryLen(file_name_1, src_file_size_1, local_file_header_offset_1);
    char* central_directory_1 = (char*) malloc(central_directory_len_1);
    BuildCentralDirectory(central_directory_1, central_directory_len_1, file_name_1, src_file_size_1,
                          local_file_header_offset_1, src_file_crc32_1);

    write(target_file_fd, central_directory_1, central_directory_len_1);
    free(central_directory_1);

    ++central_directory_count;
    total_central_directory_size += central_directory_len_1;

    // file2
    uint32_t central_directory_len_2 = GetCentralDirectoryLen(file_name_2, src_file_size_1, local_file_header_offset_2);
    char* central_directory_2 = (char*) malloc(central_directory_len_2);
    BuildCentralDirectory(central_directory_2, central_directory_len_2, file_name_2, src_file_size_1,
                          local_file_header_offset_2, src_file_crc32_1);

    write(target_file_fd, central_directory_2, central_directory_len_2);
    free(central_directory_2);

    ++central_directory_count;
    total_central_directory_size += central_directory_len_2;

    uint64_t zip64_central_directory_end_offset = first_central_directory_offset + total_central_directory_size;

    // end
    uint32_t zip64_central_directory_end_len = GetZip64CentralDirectoryEndLen();
    char* zip64_central_directory_end = (char*) malloc(zip64_central_directory_end_len);
    BuildZip64CentralDirectoryEnd(zip64_central_directory_end, zip64_central_directory_end_len, central_directory_count,
                                  total_central_directory_size, first_central_directory_offset);

    write(target_file_fd, zip64_central_directory_end, zip64_central_directory_end_len);
    free(zip64_central_directory_end);

    uint32_t zip64_central_directory_locator_len = GetZip64CentralDirectoryLocatorLen();
    char* zip64_central_directory_locator = (char*) malloc(zip64_central_directory_locator_len);
    BuildZip64CentralDirectoryLocator(zip64_central_directory_locator, zip64_central_directory_locator_len,
                                      zip64_central_directory_end_offset);

    write(target_file_fd, zip64_central_directory_locator, zip64_central_directory_locator_len);
    free(zip64_central_directory_locator);

    uint32_t central_directory_end_len = GetCentralDirectoryEndLen();
    char* central_directory_end = (char*) malloc(central_directory_end_len);
    BuildCentralDirectoryEnd(central_directory_end, central_directory_end_len, central_directory_count,
                             total_central_directory_size, first_central_directory_offset);

    write(target_file_fd, central_directory_end, central_directory_end_len);
    free(central_directory_end);

    close(target_file_fd);

    free(buf);
}

void ZipTest::Test006()
{
    char src_file_name_1[] = "./aaa.zip";

    // src file 1
    int src_file_fd_1 = open(src_file_name_1, O_RDONLY);
    if (src_file_fd_1 < 0)
    {
        FAIL();
    }

    struct stat src_file_stat_1;
    fstat(src_file_fd_1, &src_file_stat_1);
    uint64_t src_file_size_1 = src_file_stat_1.st_size;

    uint32_t src_file_crc32_1 = 0;

    uint32_t buf_size = 10 * 1024 * 1024; // 一次读10M
    char* buf = (char*) malloc(buf_size);
    uint64_t size_left = src_file_size_1;

    while (size_left > 0)
    {
        uint32_t n = read(src_file_fd_1, buf, buf_size);
        src_file_crc32_1 = FastCRC32(buf, n, src_file_crc32_1);

        if (size_left > buf_size)
        {
            size_left -= buf_size;
        }
        else
        {
            size_left = 0;
        }
    }

    close(src_file_fd_1);

    // target zip file
    int target_file_fd = open("006.zip", O_WRONLY | O_CREAT | O_TRUNC, 00666);
    if (target_file_fd < 0)
    {
        FAIL();
    }

    uint16_t central_directory_count = 0;
    uint32_t total_central_directory_size = 0;

    uint64_t local_file_header_offset_1 = 0;

    // file1
    char file_name_1[] = "test/aaa.zip";

    uint32_t local_file_header_len_1 = GetLocalFileHeaderLen(file_name_1, src_file_size_1);
    char* local_file_header_1 = (char*) malloc(local_file_header_len_1);
    BuildLocalFileHeader(local_file_header_1, local_file_header_len_1, file_name_1, src_file_size_1, src_file_crc32_1);
    write(target_file_fd, local_file_header_1, local_file_header_len_1);
    free(local_file_header_1);

    // file1 data
    src_file_fd_1 = open(src_file_name_1, O_RDONLY);
    size_left = src_file_size_1;

    while (size_left > 0)
    {
        uint32_t n = read(src_file_fd_1, buf, buf_size);
        write(target_file_fd, buf, n);

        if (size_left > buf_size)
        {
            size_left -= buf_size;
        }
        else
        {
            size_left = 0;
        }
    }

    close(src_file_fd_1);

    uint64_t local_file_header_offset_2 = local_file_header_offset_1 + local_file_header_len_1 + src_file_size_1;

    // file2
    char file_name_2[] = "1/2/";
    uint64_t file_size_2 = 0;
    uint32_t file_crc32_2 = 0;

    uint32_t local_file_header_len_2 = GetLocalFileHeaderLen(file_name_2, file_size_2);
    char* local_file_header_2 = (char*) malloc(local_file_header_len_2);
    BuildLocalFileHeader(local_file_header_2, local_file_header_len_2, file_name_2, file_size_2, file_crc32_2);
    write(target_file_fd, local_file_header_2, local_file_header_len_2);
    free(local_file_header_2);

    uint64_t first_central_directory_offset = local_file_header_offset_2 + local_file_header_len_2 + file_size_2;

    ////////////////////////////////////////////////////////////////////////////////
    // central directory

    // file1
    uint32_t central_directory_len_1 = GetCentralDirectoryLen(file_name_1, src_file_size_1, local_file_header_offset_1);
    char* central_directory_1 = (char*) malloc(central_directory_len_1);
    BuildCentralDirectory(central_directory_1, central_directory_len_1, file_name_1, src_file_size_1,
                          local_file_header_offset_1, src_file_crc32_1);

    write(target_file_fd, central_directory_1, central_directory_len_1);
    free(central_directory_1);

    ++central_directory_count;
    total_central_directory_size += central_directory_len_1;

    // file2
    uint32_t central_directory_len_2 = GetCentralDirectoryLen(file_name_2, file_size_2, local_file_header_offset_2);
    char* central_directory_2 = (char*) malloc(central_directory_len_2);
    BuildCentralDirectory(central_directory_2, central_directory_len_2, file_name_2, file_size_2,
                          local_file_header_offset_2, file_crc32_2);

    write(target_file_fd, central_directory_2, central_directory_len_2);
    free(central_directory_2);

    ++central_directory_count;
    total_central_directory_size += central_directory_len_2;

    uint64_t zip64_central_directory_end_offset = first_central_directory_offset + total_central_directory_size;

    // end
    uint32_t zip64_central_directory_end_len = GetZip64CentralDirectoryEndLen();
    char* zip64_central_directory_end = (char*) malloc(zip64_central_directory_end_len);
    BuildZip64CentralDirectoryEnd(zip64_central_directory_end, zip64_central_directory_end_len, central_directory_count,
                                  total_central_directory_size, first_central_directory_offset);

    write(target_file_fd, zip64_central_directory_end, zip64_central_directory_end_len);
    free(zip64_central_directory_end);

    uint32_t zip64_central_directory_locator_len = GetZip64CentralDirectoryLocatorLen();
    char* zip64_central_directory_locator = (char*) malloc(zip64_central_directory_locator_len);
    BuildZip64CentralDirectoryLocator(zip64_central_directory_locator, zip64_central_directory_locator_len,
                                      zip64_central_directory_end_offset);

    write(target_file_fd, zip64_central_directory_locator, zip64_central_directory_locator_len);
    free(zip64_central_directory_locator);

    uint32_t central_directory_end_len = GetCentralDirectoryEndLen();
    char* central_directory_end = (char*) malloc(central_directory_end_len);
    BuildCentralDirectoryEnd(central_directory_end, central_directory_end_len, central_directory_count,
                             total_central_directory_size, first_central_directory_offset);

    write(target_file_fd, central_directory_end, central_directory_end_len);
    free(central_directory_end);

    close(target_file_fd);

    free(buf);
}

ADD_TEST_F(ZipTest, Test001);
ADD_TEST_F(ZipTest, Test002);
ADD_TEST_F(ZipTest, Test003);
ADD_TEST_F(ZipTest, Test004);
ADD_TEST_F(ZipTest, Test005);
ADD_TEST_F(ZipTest, Test006);
