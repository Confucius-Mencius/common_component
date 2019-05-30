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
#include <aws/transfer/TransferManager.h>

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

// 异步上传
void S3Test::Test006()
{
    // Put the file into the S3 bucket asynchronously
            std::unique_lock<std::mutex> lock(upload_mutex);
            if (put_s3_object_async(s3Client,
                                    bucket_name,
                                    object_name,
                                    file_name)) {
                // While the upload is in progress, we can perform other tasks.
                // For this example, we just wait for the upload to finish.
                std::cout << "main: Waiting for file upload to complete..."
                    << std::endl;
                upload_variable.wait(lock);

                // The upload has finished. The S3Client object can be cleaned up
                // now. We can also terminate the program if we wish.
                std::cout << "main: File upload completed" << std::endl;
            }
}

// transfer方式
void S3Test::Test007()
{
    // https://aws.amazon.com/cn/blogs/aws/aws-sdk-for-c-now-ready-for-production-use/

    Aws::Transfer::TransferManagerConfiguration transferConfig;
        transferConfig.s3Client = s3Client;

        transferConfig.transferStatusUpdatedCallback =
           [](const TransferManager*, const TransferHandle& handle)
           { std::cout << "Transfer Status = " << static_cast(handle.GetStatus()) << "\n"; }

        transferConfig.uploadProgressCallback =
            [](const TransferManager*, const TransferHandle& handle)
            { std::cout << "Upload Progress: " << handle.GetBytesTransferred() << " of " << handle.GetBytesTotalSize() << " bytes\n";};

        transferConfig.downloadProgressCallback =
            [](const TransferManager*, const TransferHandle& handle)
            { std::cout << "Download Progress: " << handle.GetBytesTransferred() << " of " << handle.GetBytesTotalSize() << " bytes\n"; };

        Aws::Transfer::TransferManager transferManager(transferConfig);
        auto transferHandle = transferManager.UploadFile("/user/aws/giantFile", "aws_cpp_ga", "giantFile",
                                                         "text/plain", Aws::Map<Aws::String, Aws::String>());
        transferHandle.WaitUntilFinished();
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
    if (!outcome.IsSuccess())
    {
        auto error = outcome.GetError();
        std::cout << "ERROR: " << error.GetExceptionName() << ": "
                  << error.GetMessage() << std::endl;
        return false;
    }

    return true;
}

void S3Test::put_object_async_finished(const Aws::S3::S3Client *client, const Aws::S3::Model::PutObjectRequest &request, const Aws::S3::Model::PutObjectOutcome &outcome, const std::shared_ptr<const Aws::Client::AsyncCallerContext> &context)
{
    // Output operation status
        if (outcome.IsSuccess()) {
            std::cout << "put_object_async_finished: Finished uploading "
                << context->GetUUID() << std::endl;
        }
        else {
            auto error = outcome.GetError();
            std::cout << "ERROR: " << error.GetExceptionName() << ": "
                << error.GetMessage() << std::endl;
        }

        // Notify the thread that started the operation
        upload_variable.notify_one();
}

bool S3Test::put_s3_object_async(const Aws::S3::S3Client &s3_client, const Aws::String &s3_bucket_name, const Aws::String &s3_object_name, const std::string &file_name)
{
    // Verify file_name exists
        if (!file_exists(file_name)) {
            std::cout << "ERROR: NoSuchFile: The specified file does not exist"
                << std::endl;
            return false;
        }

        // Set up request
        Aws::S3::Model::PutObjectRequest object_request;

        object_request.SetBucket(s3_bucket_name);
        object_request.SetKey(s3_object_name);
        const std::shared_ptr<Aws::IOStream> input_data =
            Aws::MakeShared<Aws::FStream>("SampleAllocationTag",
                file_name.c_str(),
                std::ios_base::in | std::ios_base::binary);
        object_request.SetBody(input_data);

        // Set up AsyncCallerContext. Pass the S3 object name to the callback.
        auto context =
            Aws::MakeShared<Aws::Client::AsyncCallerContext>("PutObjectAllocationTag");
        context->SetUUID(s3_object_name);

        // Put the object asynchronously
        s3_client.PutObjectAsync(object_request,
                                 put_object_async_finished,
                                 context);
        return true;
}

ADD_TEST_F(S3Test, Test001)
ADD_TEST_F(S3Test, Test002)
ADD_TEST_F(S3Test, Test003)
ADD_TEST_F(S3Test, Test004)
ADD_TEST_F(S3Test, Test005)
}

