#include "sts_test.h"
#include <fstream>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/sts/model/GetSessionTokenRequest.h>

//https://docs.aws.amazon.com/zh_cn/sdk-for-java/v1/developer-guide/prog-services-sts.html

namespace aws_test
{
STSTest::STSTest()
{
    sts_client_ = nullptr;
}

STSTest::~STSTest()
{

}

void STSTest::SetUp()
{
    options_.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
    Aws::InitAPI(options_);

    Aws::Client::ClientConfiguration conf;
    conf.region = Aws::Region::AP_NORTHEAST_2;
    conf.endpointOverride = "sts.ap-northeast-2.amazonaws.com";

//    对于 IAM 用户，临时凭证的有效期范围是 900 秒 (15 分钟) 到 129600 秒 (36 小时)。如果不指定有效期，则默认使用 43200 秒 (12 小时)。
//    对于根 AWS 账户，临时凭证的有效期范围是 900 到 3600 秒 (1 小时)，如果不指定有效期，则使用默认值 3600 秒。
    Aws::Auth::AWSCredentials cred("AKIAIDDB2PURAO76V64A", "erP8zvQV6GPiZjjxmw9gM9OyKtpSbVhwk3uVCaKY");

    sts_client_ = new Aws::STS::STSClient(cred, conf);
    if (nullptr == sts_client_)
    {
        FAIL();
    }
}

void STSTest::TearDown()
{
    if (sts_client_ != nullptr)
    {
        delete sts_client_;
    }

    Aws::ShutdownAPI(options_);
}

void STSTest::Test001()
{
    Aws::STS::Model::GetSessionTokenRequest request;
    request.SetDurationSeconds(7200);

    auto outcome = sts_client_->GetSessionToken(request);
    if (!outcome.IsSuccess())
    {

    }
    else
    {
        sleep(5);
        auto credentials = outcome.GetResult().GetCredentials();
        std::cout << credentials.GetAccessKeyId() << "\n" << credentials.GetSecretAccessKey() << "\n"
                  << credentials.GetSessionToken() << "\n" << credentials.GetExpiration().Millis() / 1000 - time(NULL) << "\n" << std::endl;
    }
}

ADD_TEST_F(STSTest, Test001)
}

