#ifndef BASE_TEST_AWS_STS_TEST_H_
#define BASE_TEST_AWS_STS_TEST_H_

#include <sys/stat.h>
#include <aws/core/Aws.h>
#include <aws/sts/STSClient.h>
#include "test_util.h"

namespace aws_test
{
class STSTest : public GTest
{
public:
    STSTest();
    virtual ~STSTest();

    virtual void SetUp();
    virtual void TearDown();

    void Test001();
    void Test002();

private:
    inline bool FileExists(const std::string& name)
    {
        struct stat buffer;
        return (stat(name.c_str(), &buffer) == 0);
    }

private:
    Aws::SDKOptions options_;
    Aws::STS::STSClient* sts_client_;
};
}

#endif // BASE_TEST_AWS_STS_TEST_H_
