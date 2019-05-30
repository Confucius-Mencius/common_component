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
    void Test006();
    void Test007();

private:
    inline bool FileExists(const std::string& name)
    {
        struct stat buffer;
        return (stat(name.c_str(), &buffer) == 0);
    }

    bool PutS3Object(const Aws::String& bucket_name,
                     const Aws::String& object_key,
                     const std::string& file_path);


    /**
     * Function called when PutObjectAsync() finishes
     *
     * The thread that started the async operation is waiting for notification
     * that the operation has finished. A std::condition_variable is used to
     * communicate between the two threads.
    */
    void put_object_async_finished(const Aws::S3::S3Client* client,
        const Aws::S3::Model::PutObjectRequest& request,
        const Aws::S3::Model::PutObjectOutcome& outcome,
        const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context);

    bool put_s3_object_async(const Aws::S3::S3Client& s3_client,
        const Aws::String& s3_bucket_name,
        const Aws::String& s3_object_name,
        const std::string& file_name);

private:
    Aws::SDKOptions options_;
    Aws::S3::S3Client* s3_client_;

    std::mutex upload_mutex;
    std::condition_variable upload_variable;
};
}

#endif // BASE_TEST_AWS_S3_TEST_H_
