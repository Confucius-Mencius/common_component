#ifndef BASE_TEST_AWS_TRANSFER_TEST_H_
#define BASE_TEST_AWS_TRANSFER_TEST_H_

#include <sys/stat.h>
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include "test_util.h"

namespace aws_test
{
class TransferTest : public GTest
{
public:
    TransferTest();
    virtual ~TransferTest();

    virtual void SetUp();
    virtual void TearDown();

    void Test001();

private:
    inline bool FileExists(const std::string& name)
    {
        struct stat buffer;
        return (stat(name.c_str(), &buffer) == 0);
    }

private:
    Aws::SDKOptions options_;
    Aws::S3::S3Client* s3_client_;
};
}

#endif // BASE_TEST_AWS_TRANSFER_TEST_H_
