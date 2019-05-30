#include "transfer_test.h"
#include <fstream>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/transfer/TransferManager.h>

namespace aws_test
{
TransferTest::TransferTest()
{
    s3_client_ = nullptr;
}

TransferTest::~TransferTest()
{

}

void TransferTest::SetUp()
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

void TransferTest::TearDown()
{
    if (s3_client_ != nullptr)
    {
        delete s3_client_;
    }

    Aws::ShutdownAPI(options_);
}

void TransferTest::Test001()
{
    // https://aws.amazon.com/cn/blogs/aws/aws-sdk-for-c-now-ready-for-production-use/

//    Aws::Transfer::TransferManagerConfiguration transferConfig;
//    transferConfig.s3Client = std::make_shared<Aws::S3::S3Client>(s3_client_);

//    transferConfig.transferStatusUpdatedCallback =
//        [](const Aws::Transfer::TransferManager*, const Aws::Transfer::TransferHandle & handle)
//    {
//        std::cout << "Transfer Status = " << handle.GetStatus() << "\n";
//    }

//    transferConfig.uploadProgressCallback =
//        [](const Aws::Transfer::TransferManager*, const Aws::Transfer::TransferHandle & handle)
//    {
//        std::cout << "Upload Progress: " << handle.GetBytesTransferred() << " of " << handle.GetBytesTotalSize() << " bytes\n";
//    };

//    transferConfig.downloadProgressCallback =
//        [](const Aws::Transfer::TransferManager*, const Aws::Transfer::TransferHandle & handle)
//    {
//        std::cout << "Download Progress: " << handle.GetBytesTransferred() << " of " << handle.GetBytesTotalSize() << " bytes\n";
//    };

//    Aws::Transfer::TransferManager transferManager(transferConfig);
//    auto transferHandle = transferManager.UploadFile("/user/aws/giantFile", "aws_cpp_ga", "giantFile",
//                          "text/plain", Aws::Map<Aws::String, Aws::String>());

//    transferHandle.WaitUntilFinished();
}

ADD_TEST_F(TransferTest, Test001)
}

