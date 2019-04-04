#include "file_util_test.h"
#include <fcntl.h>
#include <utime.h>
#include "data_types.h"
#include "file_util.h"
#include "simple_log.h"
#include "str_util.h"

#define DIR_1_NAME "hello"
#define DIR_2_NAME "world"
#define FILE_NAME "me"

namespace file_util_test
{
struct SomeStruct
{
    i32 _a;
    char _b[512];
    i64 _c;
    f32 _d;
    char _e[512];
    f64 _f;
};

FileUtilTest::FileUtilTest()
{
}

FileUtilTest::~FileUtilTest()
{
}

/**
 * @brief CreateFile测试
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void FileUtilTest::Test001()
{
    const char file_path[] = "./data/" FILE_NAME;
    EXPECT_FALSE(FileExist(file_path));

    EXPECT_TRUE(CreateFile(file_path) != -1);
    EXPECT_TRUE(FileExist(file_path));

    EXPECT_EQ(0, DelFile(file_path));
    EXPECT_FALSE(FileExist(file_path));
}

/**
 * @brief CreateDir测试
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void FileUtilTest::Test002()
{
    // 一个目录
    const char file_path1[] = "./data/" DIR_1_NAME;
    EXPECT_FALSE(FileExist(file_path1));

    EXPECT_EQ(0, CreateDir(file_path1));
    EXPECT_TRUE(FileExist(file_path1));

    EXPECT_EQ(0, DelFile(file_path1));
    EXPECT_FALSE(FileExist(file_path1));

    // 多个目录嵌套
    const char file_path2[] = "./data/" DIR_1_NAME "/" DIR_2_NAME;
    EXPECT_FALSE(FileExist(file_path2));

    EXPECT_EQ(0, CreateDir(file_path2));
    EXPECT_TRUE(FileExist(file_path2));

    EXPECT_EQ(0, DelFile(file_path2));
    EXPECT_FALSE(FileExist(file_path2));

    // 上一级目录还存在
    EXPECT_TRUE(FileExist(file_path1));

    EXPECT_EQ(0, DelFile(file_path1));
    EXPECT_FALSE(FileExist(file_path1));
}

/**
 * @brief GetFileName测试
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void FileUtilTest::Test003()
{
    // 当前目录下的一个文件
    const char file_path1[] = "./data/" FILE_NAME;
    if (!FileExist(file_path1))
    {
        EXPECT_TRUE(CreateFile(file_path1) != -1);
        EXPECT_TRUE(FileExist(file_path1));
    }

    char file_name1[MAX_PATH_LEN] = "";
    EXPECT_EQ(0, GetFileName(file_name1, sizeof(file_name1), file_path1));
    EXPECT_STREQ(FILE_NAME, file_name1);

    EXPECT_EQ(0, DelFile(file_path1));
    EXPECT_FALSE(FileExist(file_path1));

    // 一个目录
    const char file_path2[] = "./data/" DIR_1_NAME;
    if (!FileExist(file_path2))
    {
        EXPECT_EQ(0, CreateDir(file_path2));
        EXPECT_TRUE(FileExist(file_path2));
    }

    char file_name2[MAX_PATH_LEN] = "";
    EXPECT_EQ(0, GetFileName(file_name2, sizeof(file_name2), file_path2));
    EXPECT_STREQ(DIR_1_NAME, file_name2);

    // 多个目录嵌套
    const char file_path3[] = "./data/" DIR_1_NAME "/" DIR_2_NAME;
    if (!FileExist(file_path3))
    {
        EXPECT_EQ(0, CreateDir(file_path3));
        EXPECT_TRUE(FileExist(file_path3));
    }

    char file_name3[MAX_PATH_LEN] = "";
    EXPECT_EQ(0, GetFileName(file_name3, sizeof(file_name3), file_path3));
    EXPECT_STREQ(DIR_2_NAME, file_name3);

    // 多个目录嵌套下的文件
    const char file_path4[] = "./data/" DIR_1_NAME "/" DIR_2_NAME "/" FILE_NAME;
    if (!FileExist(file_path4))
    {
        EXPECT_TRUE(CreateFile(file_path4) != -1);
        EXPECT_TRUE(FileExist(file_path4));
    }

    char file_name4[MAX_PATH_LEN] = "";
    EXPECT_EQ(0, GetFileName(file_name4, sizeof(file_name4), file_path4));
    EXPECT_STREQ(FILE_NAME, file_name4);

    EXPECT_EQ(0, DelFile(file_path4));
    EXPECT_FALSE(FileExist(file_path4));

    // 上一级目录
    EXPECT_TRUE(FileExist(file_path3));

    EXPECT_EQ(0, DelFile(file_path3));
    EXPECT_FALSE(FileExist(file_path3));

    // 上一级目录
    EXPECT_TRUE(FileExist(file_path2));

    EXPECT_EQ(0, DelFile(file_path2));
    EXPECT_FALSE(FileExist(file_path2));
}

/**
 * @brief GetDirName测试
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void FileUtilTest::Test004()
{
    // 当前目录下的一个文件
    const char file_path1[] = "./data/" FILE_NAME;
    if (!FileExist(file_path1))
    {
        EXPECT_TRUE(CreateFile(file_path1) != -1);
        EXPECT_TRUE(FileExist(file_path1));
    }

    char file_dir1[MAX_PATH_LEN] = "";
    EXPECT_EQ(0, GetFileDir(file_dir1, sizeof(file_dir1), file_path1));
    EXPECT_STREQ("./data", file_dir1);

    EXPECT_EQ(0, DelFile(file_path1));
    EXPECT_FALSE(FileExist(file_path1));

    // 一个目录
    const char file_path2[] = "./data/" DIR_1_NAME;
    if (!FileExist(file_path2))
    {
        EXPECT_EQ(0, CreateDir(file_path2));
        EXPECT_TRUE(FileExist(file_path2));
    }

    char file_dir2[MAX_PATH_LEN] = "";
    EXPECT_EQ(0, GetFileDir(file_dir2, sizeof(file_dir2), file_path2));
    EXPECT_STREQ("./data", file_dir2);

    // 多个目录嵌套
    const char file_path3[] = "./data/" DIR_1_NAME "/" DIR_2_NAME;
    if (!FileExist(file_path3))
    {
        EXPECT_EQ(0, CreateDir(file_path3));
        EXPECT_TRUE(FileExist(file_path3));
    }

    char file_dir3[MAX_PATH_LEN] = "";
    EXPECT_EQ(0, GetFileDir(file_dir3, sizeof(file_dir3), file_path3));
    EXPECT_STREQ(file_path2, file_dir3);

    // 多个目录嵌套下的文件
    const char file_path4[] = "./data/" DIR_1_NAME "/" DIR_2_NAME "/" FILE_NAME;
    if (!FileExist(file_path4))
    {
        EXPECT_TRUE(CreateFile(file_path4) != -1);
        EXPECT_TRUE(FileExist(file_path4));
    }

    char file_dir4[MAX_PATH_LEN] = "";
    EXPECT_EQ(0, GetFileDir(file_dir4, sizeof(file_dir4), file_path4));
    EXPECT_STREQ(file_path3, file_dir4);

    EXPECT_EQ(0, DelFile(file_path4));
    EXPECT_FALSE(FileExist(file_path4));

    // 上一级目录
    EXPECT_TRUE(FileExist(file_path3));

    EXPECT_EQ(0, DelFile(file_path3));
    EXPECT_FALSE(FileExist(file_path3));

    // 上一级目录
    EXPECT_TRUE(FileExist(file_path2));

    EXPECT_EQ(0, DelFile(file_path2));
    EXPECT_FALSE(FileExist(file_path2));
}

/**
 * @brief WriteBinFile测试
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void FileUtilTest::Test005()
{
    const char file_path[] = "./data/shfslfjl.bin";

    SomeStruct in;
    const size_t in_len = sizeof(in);
    memset(&in, 0, in_len);

    ASSERT_EQ(0, WriteBinFile(file_path, &in, in_len));

    SomeStruct out;
    size_t out_len = in_len;
    ASSERT_EQ(0, ReadBinFile(&out, out_len, file_path));

    EXPECT_EQ(in._a, out._a);
    EXPECT_STREQ(in._b, out._b);
    EXPECT_EQ(in._c, out._c);
    EXPECT_EQ(in._d, out._d);
    EXPECT_STREQ(in._e, out._e);
    EXPECT_EQ(in._f, out._f);
}

/**
 * @brief WriteTxtFile测试
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void FileUtilTest::Test006()
{
    const char file_path[] = "./data/ahfglajfl.txt";

    const char in[] = "dflafjlsad  fal34- 04flafj d-02-43";
    const size_t in_len = StrLen(in, sizeof(in));
    ASSERT_EQ(0, WriteTxtFile(file_path, in, in_len));

    char out[512] = "";
    size_t out_len = in_len;
    ASSERT_EQ(0, ReadTxtFile(out, out_len, file_path));
    EXPECT_STREQ(in, out);
}

void FileUtilTest::Test007()
{
    char file_name[] = "./data/123.txt";
    int fd = creat(file_name, S_IRWXU | S_IRWXG);
    ASSERT_TRUE(fd != -1);

    fd = creat("./data/abc.txt", S_IRWXU | S_IRWXG);
    ASSERT_TRUE(fd != -1);

    struct stat stat_info;
    int ret = stat(file_name, &stat_info);
    ASSERT_TRUE(0 == ret);

    LOG_CPP(stat_info.st_atime);   /* time of last access */
    LOG_CPP(stat_info.st_mtime);   /* time of last modification */
    LOG_CPP(stat_info.st_ctime);   /* time of last status change */

    sleep(1);
    time_t now = time(nullptr);

    struct utimbuf utim_info;
    utim_info.actime = now;
    utim_info.modtime = now;
    ret = utime(file_name, &utim_info);
    ASSERT_TRUE(0 == ret);

    ret = stat(file_name, &stat_info);
    ASSERT_TRUE(0 == ret);

    LOG_CPP(stat_info.st_atime);   /* time of last access */
    LOG_CPP(stat_info.st_mtime);   /* time of last modification */
    LOG_CPP(stat_info.st_ctime);   /* time of last status change */
}

/**
 * @brief 连同目录一起创建文件
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void FileUtilTest::Test008()
{
    // 多个目录嵌套下的文件
    const char file_path[] = "./data/" DIR_1_NAME "/" DIR_2_NAME "/" FILE_NAME;
    if (!FileExist(file_path))
    {
        EXPECT_TRUE(CreateFile(file_path) != -1);
        EXPECT_TRUE(FileExist(file_path));
    }

    char file_dir2[MAX_PATH_LEN] = "";
    EXPECT_EQ(0, GetFileDir(file_dir2, sizeof(file_dir2), file_path));
    EXPECT_TRUE(FileExist(file_dir2));
    EXPECT_EQ(0, DelFile(file_dir2));
    EXPECT_FALSE(FileExist(file_dir2));

    char file_dir1[MAX_PATH_LEN] = "";
    EXPECT_EQ(0, GetFileDir(file_dir1, sizeof(file_dir1), file_dir2));
    EXPECT_TRUE(FileExist(file_dir1));
    EXPECT_EQ(0, DelFile(file_dir1));
    EXPECT_FALSE(FileExist(file_dir1));
}

void FileUtilTest::Test009()
{
    char buf[MAX_PATH_LEN] = "";
    ASSERT_EQ(0, GetAbsolutePath(buf, sizeof(buf), ".", "/"));
    LOG_CPP(buf);

    ASSERT_EQ(0, GetAbsolutePath(buf, sizeof(buf), ".", "/home"));
    LOG_CPP(buf);

    ASSERT_EQ(0, GetAbsolutePath(buf, sizeof(buf), "../xx", "/home"));
    LOG_CPP(buf);

    char cur_working_dir[] = "/home";
    ASSERT_EQ(0, GetAbsolutePath(buf, sizeof(buf), "../xx", cur_working_dir));
    LOG_CPP(buf);

    ASSERT_EQ(0, GetAbsolutePath(buf, sizeof(buf), "./www", "/home/"));
    LOG_CPP(buf);

    ASSERT_EQ(0, GetAbsolutePath(buf, sizeof(buf), "./www/", "/home/"));
    LOG_CPP(buf);

    ASSERT_EQ(0, GetAbsolutePath(buf, sizeof(buf), "/www/", "/home/"));
    LOG_CPP(buf);
}

ADD_TEST_F(FileUtilTest, Test001);
ADD_TEST_F(FileUtilTest, Test002);
ADD_TEST_F(FileUtilTest, Test003);
ADD_TEST_F(FileUtilTest, Test004);
ADD_TEST_F(FileUtilTest, Test005);
ADD_TEST_F(FileUtilTest, Test006);
ADD_TEST_F(FileUtilTest, Test007);
ADD_TEST_F(FileUtilTest, Test008);
ADD_TEST_F(FileUtilTest, Test009);
}
