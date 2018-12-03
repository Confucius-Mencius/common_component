#include "addr_port_util_test.h"
#include "addr_port_util.h"

AddrPortUtilTest::AddrPortUtilTest()
{

}

AddrPortUtilTest::~AddrPortUtilTest()
{

}

void AddrPortUtilTest::Test001()
{
    std::string addr_port = "127.0.0.1:8000";

    char addr1[10] = ""; // strlen(127.0.0.1) = 9
    unsigned short port = 0;

    if (ParseAddrPort(addr1, sizeof(addr1), port, addr_port.c_str(), addr_port.length()) != 0)
    {
        FAIL();
    }

    std::cout << addr1 << ":" << port << std::endl;

    ////////////////////////////////////////////////////////////////////////////////
    addr_port = "www.dssp.moon.com:8000";
    char addr2[32] = "";

    if (ParseAddrPort(addr2, sizeof(addr2), port, addr_port.c_str(), addr_port.length()) != 0)
    {
        FAIL();
    }

    std::cout << addr2 << ":" << port << std::endl;

    ////////////////////////////////////////////////////////////////////////////////
    addr_port = "127.0.0.1";
    EXPECT_TRUE(ParseAddrPort(addr1, sizeof(addr1), port, addr_port.c_str(), addr_port.length()) != 0);

    ////////////////////////////////////////////////////////////////////////////////
    addr_port = "127.0.0.1:";
    EXPECT_TRUE(ParseAddrPort(addr1, sizeof(addr1), port, addr_port.c_str(), addr_port.length()) != 0);

    ////////////////////////////////////////////////////////////////////////////////
    addr_port = ":80";
    EXPECT_TRUE(ParseAddrPort(addr1, sizeof(addr1), port, addr_port.c_str(), addr_port.length()) != 0);

    ////////////////////////////////////////////////////////////////////////////////
    addr_port = ":";
    EXPECT_TRUE(ParseAddrPort(addr1, sizeof(addr1), port, addr_port.c_str(), addr_port.length()) != 0);
}

void AddrPortUtilTest::Test002()
{
    std::string addr_port = "127.0.0.1:8000";

    char addr1[9] = "";
    unsigned short port = 0;

    int ret = ParseAddrPort(addr1, sizeof(addr1), port, addr_port.c_str(), addr_port.length());
    EXPECT_TRUE(ret != 0);

    char addr2[10] = "";
    port = 0;

    if (ParseAddrPort(addr2, sizeof(addr2), port, addr_port.c_str(), addr_port.length()) != 0)
    {
        FAIL();
    }

    std::cout << addr2 << ":" << port << std::endl;

    char addr3[64];
    port = 0;

    if (ParseAddrPort(addr3, sizeof(addr3), port, addr_port.c_str(), addr_port.length()) != 0)
    {
        FAIL();
    }

    std::cout << addr3 << ":" << port << std::endl;
}

void AddrPortUtilTest::Test003()
{
    std::string addr_port = "127.0.0.1:8000";

    char addr1[10] = "";
    unsigned short port = 0;

    if (ParseHostPort(addr1, sizeof(addr1), port, addr_port) != 0)
    {
        FAIL();
    }

    std::cout << addr1 << ":" << port << std::endl;

    ////////////////////////////////////////////////////////////////////////////////
    addr_port = "www.dssp.moon.com:8000";
    char addr2[32] = "";

    if (ParseHostPort(addr2, sizeof(addr2), port, addr_port) != 0)
    {
        FAIL();
    }

    std::cout << addr1 << ":" << port << std::endl;

    ////////////////////////////////////////////////////////////////////////////////
    addr_port = "127.0.0.1";
    EXPECT_TRUE(ParseHostPort(addr1, sizeof(addr1), port, addr_port) != 0);

    ////////////////////////////////////////////////////////////////////////////////
    addr_port = "127.0.0.1:";
    EXPECT_TRUE(ParseHostPort(addr1, sizeof(addr1), port, addr_port) != 0);

    ////////////////////////////////////////////////////////////////////////////////
    addr_port = ":80";
    EXPECT_TRUE(ParseHostPort(addr1, sizeof(addr1), port, addr_port) != 0);

    ////////////////////////////////////////////////////////////////////////////////
    addr_port = ":";
    EXPECT_TRUE(ParseHostPort(addr1, sizeof(addr1), port, addr_port) != 0);
}

void AddrPortUtilTest::Test004()
{
    std::string addr_port = "127.0.0.1:8000";

    char addr1[9] = "";
    unsigned short port = 0;

    int ret = ParseHostPort(addr1, sizeof(addr1), port, addr_port);
    EXPECT_TRUE(ret != 0);

    char addr2[10] = "";
    port = 0;

    if (ParseHostPort(addr2, sizeof(addr2), port, addr_port) != 0)
    {
        FAIL();
    }

    std::cout << addr2 << ":" << port << std::endl;

    char addr3[64];
    port = 0;

    if (ParseHostPort(addr3, sizeof(addr3), port, addr_port) != 0)
    {
        FAIL();
    }

    std::cout << addr3 << ":" << port << std::endl;
}

ADD_TEST_F(AddrPortUtilTest, Test001);
ADD_TEST_F(AddrPortUtilTest, Test002);
ADD_TEST_F(AddrPortUtilTest, Test003);
ADD_TEST_F(AddrPortUtilTest, Test004);
