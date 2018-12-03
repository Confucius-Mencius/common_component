#include "misc_test.h"
#include "misc_util.h"

MiscTest::MiscTest()
{

}

MiscTest::~MiscTest()
{

}

void MiscTest::LittleEndianTest()
{
    EXPECT_TRUE(IsLittleEndian());
}

void MiscTest::atoi_Test001()
{
    EXPECT_EQ(0, atoi(""));
//    EXPECT_EQ(0, atoi(NULL)); // SIGSEGV
    EXPECT_EQ(123, atoi("0123"));
    EXPECT_EQ(0, atoi("x123"));
}

void MiscTest::ExecShellCmdTest()
{
    char buf[4096] = "";
    ASSERT_EQ(0, ExecShellCmd(buf, sizeof(buf), "ls -al"));
    std::cout << buf << std::endl;

    ASSERT_EQ(0, ExecShellCmd(buf, sizeof(buf), "pwd"));
    std::cout << buf << std::endl;

    ASSERT_EQ(0, ExecShellCmd(buf, sizeof(buf), "echo $HOSTNAME"));
    std::cout << buf << std::endl;
}

ADD_TEST_F(MiscTest, LittleEndianTest);
ADD_TEST_F(MiscTest, atoi_Test001);
ADD_TEST_F(MiscTest, ExecShellCmdTest);
