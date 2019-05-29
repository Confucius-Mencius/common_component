#include "s3_test.h"
#include <fstream>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/model/Bucket.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/Object.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/MultipartUpload.h>

//https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/examples-s3.html

//AWSAccessKeyId=AKIAIDDB2PURAO76V64A
//AWSSecretKey=erP8zvQV6GPiZjjxmw9gM9OyKtpSbVhwk3uVCaKY

// TODO 有个异步的例子
//http://www.mindg.cn/?p=363

namespace aws_test
{
S3Test::S3Test()
{
    s3_client_ = nullptr;
}

S3Test::~S3Test()
{

}

void S3Test::SetUp()
{
    options_.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
    Aws::InitAPI(options_);

    Aws::Client::ClientConfiguration conf;
    conf.region = Aws::Region::AP_NORTHEAST_2;
    conf.endpointOverride = "s3.ap-northeast-2.amazonaws.com";

    Aws::Auth::AWSCredentials cred("AKIAIDDB2PURAO76V64A", "erP8zvQV6GPiZjjxmw9gM9OyKtpSbVhwk3uVCaKY");

    s3_client_ = new Aws::S3::S3Client(cred, conf, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);
    if (nullptr == s3_client_)
    {
        FAIL();
    }
}

void S3Test::TearDown()
{
    if (s3_client_ != nullptr)
    {
        delete s3_client_;
    }

    Aws::ShutdownAPI(options_);
}

// 列出桶
void S3Test::Test001()
{
    auto outcome = s3_client_->ListBuckets();
    if (outcome.IsSuccess())
    {
        auto buckets = outcome.GetResult().GetBuckets();
        for (auto const& bucket : buckets)
        {
            std::cout << bucket.GetName() << "\t" << bucket.GetCreationDate().ToLocalTimeString(Aws::Utils::DateFormat::RFC822) << std::endl;
        }
    }
    else
    {
        std::cout << "ListBuckets error: "
                  << outcome.GetError().GetExceptionName() << " - "
                  << outcome.GetError().GetMessage() << std::endl;
    }

}

// 上传文件
void S3Test::Test002()
{
    // Assign these values before running the program
    const Aws::String bucket_name = "store.cxsw3d";
    const Aws::String object_key = "cpp1";
    const std::string file_path = "./test_server_conf.xml";

    // Put the file into the S3 bucket
    if (PutS3Object(bucket_name, object_key, file_path))
    {
        std::cout << "Put file " << file_path
                  << " to S3 bucket " << bucket_name
                  << " as object " << object_key << std::endl;
    }
}

// 列出文件
void S3Test::Test003()
{
    const Aws::String bucket_name = "store.cxsw3d";
    std::cout << "Objects in S3 bucket: " << bucket_name << std::endl;

    Aws::S3::Model::ListObjectsRequest object_request;
    object_request.WithBucket(bucket_name);

    auto outcome = s3_client_->ListObjects(object_request);
    if (outcome.IsSuccess())
    {
        Aws::Vector<Aws::S3::Model::Object> objects =
            outcome.GetResult().GetContents();

        for (auto const& object : objects)
        {
            std::cout << "* " << object.GetKey() << " " << object.GetSize() << std::endl;
        }
    }
    else
    {
        std::cout << "ListObjects error: " <<
                  outcome.GetError().GetExceptionName() << " " <<
                  outcome.GetError().GetMessage() << std::endl;
    }
}

// 下载文件
void S3Test::Test004()
{
    // Assign these values before running the program
    const Aws::String bucket_name = "store.cxsw3d";
    const Aws::String object_key = "cpp1";  // For demo, set to a text file

    Aws::S3::Model::GetObjectRequest object_request;
    object_request.WithBucket(bucket_name).WithKey(object_key);

    // Get the object
    auto outcome = s3_client_->GetObject(object_request);
    if (outcome.IsSuccess())
    {
        // Get an Aws::IOStream reference to the retrieved file
        auto& retrieved_file = outcome.GetResultWithOwnership().GetBody();

        // Output the first line of the retrieved text file
        std::cout << "Beginning of file contents:\n";
        char file_data[255] = { 0 };
        retrieved_file.getline(file_data, 254);
        std::cout << file_data << std::endl;
    }
    else
    {
        auto error = outcome.GetError();
        std::cout << "ERROR: " << error.GetExceptionName() << ": "
                  << error.GetMessage() << std::endl;
    }
}

// 删除文件
void S3Test::Test005()
{
    const Aws::String bucket_name = "store.cxsw3d";
    const Aws::String object_key  = "cpp1";

    std::cout << "Deleting " << object_key << " from S3 bucket: " <<
              bucket_name << std::endl;

    Aws::S3::Model::DeleteObjectRequest object_request;
    object_request.WithBucket(bucket_name).WithKey(object_key);

    auto outcome = s3_client_->DeleteObject(object_request);
    if (outcome.IsSuccess())
    {
        std::cout << "Done!" << std::endl;
    }
    else
    {
        std::cout << "DeleteObject error: " <<
                  outcome.GetError().GetExceptionName() << " " <<
                  outcome.GetError().GetMessage() << std::endl;
    }
}

bool S3Test::PutS3Object(const Aws::String& bucket_name, const Aws::String& object_key, const std::string& file_path)
{
    // Verify file_name exists
    if (!file_exists(file_path))
    {
        std::cout << "ERROR: NoSuchFile: The specified file does not exist"
                  << std::endl;
        return false;
    }

    Aws::S3::Model::PutObjectRequest object_request;

    object_request.WithBucket(bucket_name).WithKey(object_key);

    const std::shared_ptr<Aws::IOStream> input_data =
        Aws::MakeShared<Aws::FStream>("CXSW3DAllocationTag",
                                      file_path.c_str(),
                                      std::ios_base::in | std::ios_base::binary);
    object_request.SetBody(input_data);

    // Put the object
    auto outcome = s3_client_->PutObject(object_request);
    if (!outcome.IsSuccess())
    {
        auto error = outcome.GetError();
        std::cout << "ERROR: " << error.GetExceptionName() << ": "
                  << error.GetMessage() << std::endl;
        return false;
    }

    return true;
}

ADD_TEST_F(S3Test, Test001)
ADD_TEST_F(S3Test, Test002)
ADD_TEST_F(S3Test, Test003)
ADD_TEST_F(S3Test, Test004)
ADD_TEST_F(S3Test, Test005)
}

