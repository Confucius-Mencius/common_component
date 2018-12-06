#include "shell_util_test.h"
#include "file_util.h"
#include "shell_util.h"

#define LOG_WITH_CONTEXT 0
#include "simple_log.h"

#include "str_util.h"

namespace shell_util_test
{
ShellUtilTest::ShellUtilTest()
{
}

ShellUtilTest::~ShellUtilTest()
{
}

void ShellUtilTest::Test001()
{
    char buf[4096] = "";
    ASSERT_EQ(0, ExecShellCmd(buf, sizeof(buf), "ls -al"));
    LOG_CPP(buf);

    ASSERT_EQ(0, ExecShellCmd(buf, sizeof(buf), "pwd"));
    LOG_CPP(buf);

    ASSERT_EQ(0, ExecShellCmd(buf, sizeof(buf), "echo $HOSTNAME"));
    LOG_CPP(buf);
}

// 结果超过4k
void ShellUtilTest::Test002()
{
    const char* test_file_path = "./data/shell_cat_file";
    const int file_len = 8192;

    DelFile(test_file_path);
    ASSERT_NE(-1, CreateFile(test_file_path));

    for (int i = 0; i < file_len; ++i)
    {
        AppendTxtFile(test_file_path, "x", 1);
    }

    char cmd[64] = "";
    StrPrintf(cmd, sizeof(cmd), "cat %s", test_file_path);

    char buf[file_len];
    ASSERT_EQ(0, ExecShellCmd(buf, sizeof(buf), cmd));
    LOG_CPP(buf);

    // buf的最后一个是'\0'
    EXPECT_EQ('\0', buf[sizeof(buf) - 1]);
    EXPECT_EQ(file_len - 1, (int) strlen(buf));
}

ADD_TEST_F(ShellUtilTest, Test001);
ADD_TEST_F(ShellUtilTest, Test002);
}
