#ifndef BASE_TEST_AWS_S3_TEST_H_
#define BASE_TEST_AWS_S3_TEST_H_

#include <sys/stat.h>
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include "test_util.h"

namespace aws_test
{
class S3Test : public GTest
{
public:
    S3Test();
    virtual ~S3Test();

    virtual void SetUp();
    virtual void TearDown();

    void Test001();
    void Test002();
    void Test003();
    void Test004();
    void Test005();

private:
    inline bool file_exists(const std::string& name)
    {
        struct stat buffer;
        return (stat(name.c_str(), &buffer) == 0);
    }

    bool PutS3Object(const Aws::String& bucket_name,
                     const Aws::String& object_key,
                     const std::string& file_path);


private:
    Aws::SDKOptions options_;
    Aws::S3::S3Client* s3_client_;
};
}

#endif // BASE_TEST_AWS_S3_TEST_H_
