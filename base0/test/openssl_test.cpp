#include "openssl_test.h"
#include "openssl/sha.h"
#include "openssl/crypto.h"
#include "boost/uuid/detail/sha1.hpp"
#include "md5_util.h"
#include "sha_util.h"

OpensslTest::OpensslTest()
{
}

OpensslTest::~OpensslTest()
{
}

const char* orgStr = "easydown:UjF5QjRfX3DdYm3Z"; //待哈希的串

// 打印前， 有必要转换
void printHash(unsigned char* md, int len)
{
    int i = 0;
    for (i = 0; i < len; i++)
    {
        printf("%02x", md[i]);
    }

    printf("\n");
}

void mySha1_0()
{
    unsigned char md[SHA_DIGEST_LENGTH + 1];
    memset(md, 0, SHA_DIGEST_LENGTH + 1);

    SHA1((unsigned char*) orgStr, strlen(orgStr), md);
    printHash(md, SHA_DIGEST_LENGTH);

    memset(md, 0, SHA_DIGEST_LENGTH + 1);

    SHA_CTX c;
    if (!SHA1_Init(&c))
    {
        return;
    }

    SHA1_Update(&c, orgStr, strlen(orgStr));
    SHA1_Final(md, &c);
    OPENSSL_cleanse(&c, sizeof(c));
    printf("%s", md);
    printHash(md, SHA_DIGEST_LENGTH);
}

void mySha1_1()
{
    const char user_pass[] = "easydown:UjF5QjRfX3DdYm3Z";
    boost::uuids::detail::sha1 sha;
    sha.process_bytes(user_pass, strlen(user_pass));
    unsigned int digest[5] = {0};
    sha.get_digest(digest);

    std::string str_sha1;
    for (int i = 0; i < 5; ++i)
    {
        std::cout << std::hex << digest[i] << std::endl;
        str_sha1.append(1, static_cast<char>((digest[i] >> 24) & 0xFF));
        str_sha1.append(1, static_cast<char>((digest[i] >> 16) & 0xFF));
        str_sha1.append(1, static_cast<char>((digest[i] >> 8) & 0xFF));
        str_sha1.append(1, static_cast<char>((digest[i]) & 0xFF));
    }

    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
    {
        printf("%02x", (unsigned char) str_sha1[i]);
    }

    std::cout << std::endl;
}

void OpensslTest::sha1_test()
{
    mySha1_0();
    mySha1_1();
}

void OpensslTest::Test001()
{
    const unsigned char data[] = "easydown:UjF5QjRfX3DdYm3Z";

    char md5[MD5_STR_LEN + 1] = "";
    CalcMD5(md5, sizeof(md5), data, strlen((const char*) data));
    std::cout << md5 << std::endl;

    char sha1[SHA1_STR_LEN + 1] = "";
    CalcSHA1(sha1, sizeof(sha1), data, strlen((const char*) data));
    std::cout << sha1 << std::endl;
}

ADD_TEST_F(OpensslTest, sha1_test);
ADD_TEST_F(OpensslTest, Test001);
