#include "s3_test.h"
#include <fstream>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/model/Bucket.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/Object.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>

//https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/examples-s3.html

//AWSAccessKeyId=AKIAIDDB2PURAO76V64A
//AWSSecretKey=erP8zvQV6GPiZjjxmw9gM9OyKtpSbVhwk3uVCaKY

// TODO 有个异步的例子
//http://www.mindg.cn/?p=363

namespace aws_test
{
std::condition_variable S3Test::upload_variable_;

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

//  Aws::Auth::AWSCredentials cred("AKIAIDDB2PURAO76V64A", "erP8zvQV6GPiZjjxmw9gM9OyKtpSbVhwk3uVCaKY");
    Aws::Auth::AWSCredentials cred("ASIAT55XXI3L2HWWFBZ7", "nGrQUg6vKHfnMpPuQ7aSIuEM9LkdYr+tR5rSxt2K", "AgoGb3JpZ2luEMj//////////wEaDmFwLW5vcnRoZWFzdC0yIoACmXcqDHh05P2P8VlZUT/Sw2zGiVAlOhfQkyAjfZIHUoaErzTPlj1G5RhRpqVOS4clenxdWPJSJaMS0FVh1e/YpI98S0787+YAQMGZrajxwXK5eclqcaLV74Cbl3KRKh6PSM7mbtdGjGEhfsvt+XNDy4E8vYp6FGHH667wfLTpgLG/iJ4+iKCpu6cA4Jk7KAnrXEJ5QE/K/yNozfk+8hXeMIwgWOwOkR5PUK2d1h8Ba7HsVTxyRXMVqZV4cIT1OdMT7HEK2Mj54Y2L7Wz2lR58QYr8p1pGaQyVhIziL1lKz1uKlbqaSJE+AVFNrL0UreYHOkxfJnaMrb8yXu5lQPNXdCq3AQh1EAAaDDI3MDQzMTM3MzAxNSIMkGpiOKaBsdIGMTV0KpQBDAlkueJJNXu+AM6g59RbF9p7lzJ/EDE4Z9XLnP12sXcYvjVfIRBdcZDeviJE7l0GXcXL46xHOoFEsc7adSRn1oSmyXboLl51RVJ55LzWDflbChh0N4k25cXfr6ZSf61pT1VNDY3ng5kpx1yXTfJoKZ3S3ASXzoCZx3pApC39WlfAr+JL6doiXgVkeFI/M4+YQyrOMzCxhr/nBQ==");

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

// 异步上传
void S3Test::Test006()
{
    const Aws::String bucket_name = "store.cxsw3d";
    const Aws::String object_key = "cpp1";
    const std::string file_path = "./test_server_conf.xml";

    // Put the file into the S3 bucket asynchronously
    std::unique_lock<std::mutex> lock(upload_mutex_);
    if (PutS3ObjectAsync(bucket_name,
                         object_key,
                         file_path))
    {
        // While the upload is in progress, we can perform other tasks.
        // For this example, we just wait for the upload to finish.
        std::cout << "main: Waiting for file upload to complete..."
                  << std::endl;
        upload_variable_.wait(lock);

        // The upload has finished. The S3Client object can be cleaned up
        // now. We can also terminate the program if we wish.
        std::cout << "main: File upload completed" << std::endl;
    }
}

bool S3Test::PutS3Object(const Aws::String& bucket_name, const Aws::String& object_key, const std::string& file_path)
{
    // Verify file_name exists
    if (!FileExists(file_path))
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
    if (!outcome.IsSuccess()) // TODO 根据错误类型做重试
    {
        auto error = outcome.GetError();

        std::cout << "ERROR: " << error.GetExceptionName() << ", "
                  << error.GetMessage() << ", " << (int) error.GetResponseCode() << std::endl;
        return false;
    }

    std::cout << outcome.GetResult().GetVersionId() << std::endl; // url: https://s3.ap-northeast-2.amazonaws.com/store.cxsw3d/Hello.stl

    return true;
}

void S3Test::PutObjectAsyncFinished(const Aws::S3::S3Client* client, const Aws::S3::Model::PutObjectRequest& request,
                                    const Aws::S3::Model::PutObjectOutcome& outcome, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context)
{
    // Output operation status
    if (outcome.IsSuccess())
    {
        std::cout << "put_object_async_finished: Finished uploading "
                  << context->GetUUID() << std::endl;
    }
    else
    {
        auto error = outcome.GetError();
        std::cout << "ERROR: " << error.GetExceptionName() << ": "
                  << error.GetMessage() << std::endl;
    }

    // Notify the thread that started the operation
    upload_variable_.notify_one();
}

bool S3Test::PutS3ObjectAsync(const Aws::String& bucket_name, const Aws::String& object_key, const std::string& file_path)
{
    // Verify file_name exists
    if (!FileExists(file_path))
    {
        std::cout << "ERROR: NoSuchFile: The specified file does not exist"
                  << std::endl;
        return false;
    }

    // Set up request
    Aws::S3::Model::PutObjectRequest object_request;
    object_request.WithBucket(bucket_name).WithKey(object_key);

    const std::shared_ptr<Aws::IOStream> input_data =
        Aws::MakeShared<Aws::FStream>("SampleAllocationTag",
                                      file_path.c_str(),
                                      std::ios_base::in | std::ios_base::binary);
    object_request.SetBody(input_data);

    // Set up AsyncCallerContext. Pass the S3 object name to the callback.
    auto context = Aws::MakeShared<Aws::Client::AsyncCallerContext>("PutObjectAllocationTag");
    context->SetUUID(object_key);

    // Put the object asynchronously
    s3_client_->PutObjectAsync(object_request,
                               S3Test::PutObjectAsyncFinished,
                               context);
    return true;
}

ADD_TEST_F(S3Test, Test001)
ADD_TEST_F(S3Test, Test002)
ADD_TEST_F(S3Test, Test003)
ADD_TEST_F(S3Test, Test004)
ADD_TEST_F(S3Test, Test005)
ADD_TEST_F(S3Test, Test006)
}

