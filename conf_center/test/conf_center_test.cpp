#include "conf_center_test.h"
#include "mem_util.h"

static const char app_conf_file_path[] = "./test_server_conf.xml";

ConfCenterTest::ConfCenterTest()
{
    conf_center_ = NULL;
}

ConfCenterTest::~ConfCenterTest()
{
}

void ConfCenterTest::SetUp()
{
    if (loader_.Load("../libconf_center.so") != 0)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    conf_center_ = (ConfCenterInterface*) loader_.GetModuleInterface(0);
    if (NULL == conf_center_)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    std::cout << "conf center version: " << conf_center_->GetVersion() << std::endl;

    ConfCenterCtx conf_center_ctx;
    conf_center_ctx.app_conf_file_path = app_conf_file_path;

    if (conf_center_->Initialize(&conf_center_ctx) != 0)
    {
        FAIL() << conf_center_->GetLastErrMsg();
    }

    if (conf_center_->Activate() != 0)
    {
        FAIL() << conf_center_->GetLastErrMsg();
    }
}

void ConfCenterTest::TearDown()
{
    SAFE_DESTROY_MODULE(conf_center_, loader_);
}

/**
 * @brief 取单条配置，包括配置在node和attrib两种情况
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void ConfCenterTest::Test001()
{
    // 取单个值的节点
    ////////////////////////////////////////////////////////////////////////////////
    i32 port;

    int ret = conf_center_->GetConf(port, "/conf/port");
    EXPECT_EQ(0, ret);
    std::cout << "port: " << port << std::endl;

    ret = conf_center_->GetConf(port, "/conf/port");
    EXPECT_EQ(0, ret);
    std::cout << "port: " << port << std::endl;

    ////////////////////////////////////////////////////////////////////////////////
    f32 coef;

    ret = conf_center_->GetConf(coef, "/conf/coef");
    EXPECT_EQ(0, ret);
    std::cout << "coef: " << coef << std::endl;

    ret = conf_center_->GetConf(coef, "/conf/coef");
    EXPECT_EQ(0, ret);
    std::cout << "coef: " << coef << std::endl;

    ////////////////////////////////////////////////////////////////////////////////
    char* ip = NULL;

    ret = conf_center_->GetConf(&ip, "/conf/ip");
    EXPECT_EQ(0, ret);
    std::cout << "ip: " << ip << std::endl;
    conf_center_->ReleaseConf(&ip);

    ret = conf_center_->GetConf(&ip, "/conf/ip");
    EXPECT_EQ(0, ret);
    std::cout << "ip: " << ip << std::endl;
    conf_center_->ReleaseConf(&ip);

    ////////////////////////////////////////////////////////////////////////////////
    // 取属性
    ////////////////////////////////////////////////////////////////////////////////
    i64 nthreads;

    ret = conf_center_->GetConf(nthreads, "/conf/db/@threads");
    EXPECT_EQ(0, ret);
    std::cout << "nthreads: " << nthreads << std::endl;

    ret = conf_center_->GetConf(nthreads, "/conf/db/@threads", true, 16);
    EXPECT_EQ(0, ret);
    std::cout << "nthreads: " << nthreads << std::endl;

    ////////////////////////////////////////////////////////////////////////////////
    f64 attrib_coef;

    ret = conf_center_->GetConf(attrib_coef, "/conf/db/@coef", true, 1.0);
    EXPECT_EQ(0, ret);
    std::cout << "attrib_coef: " << attrib_coef << std::endl;

    ret = conf_center_->GetConf(attrib_coef, "/conf/db/@coef", true, 1.0);
    EXPECT_EQ(0, ret);
    std::cout << "attrib_coef: " << attrib_coef << std::endl;

    ////////////////////////////////////////////////////////////////////////////////
    char* db_name = NULL;

    ret = conf_center_->GetConf(&db_name, "/conf/db/@name");
    EXPECT_EQ(0, ret);
    std::cout << "db_name: " << db_name << std::endl;
    conf_center_->ReleaseConf(&db_name);

    ret = conf_center_->GetConf(&db_name, "/conf/db/@name");
    EXPECT_EQ(0, ret);
    std::cout << "db_name: " << db_name << std::endl;
    conf_center_->ReleaseConf(&db_name);

    ////////////////////////////////////////////////////////////////////////////////
    // 不存在的节点
    ////////////////////////////////////////////////////////////////////////////////
    i64 not_exist_i64;

    ret = conf_center_->GetConf(not_exist_i64, "/conf/not_exist_node");
    EXPECT_TRUE(ret != 0);

    ret = conf_center_->GetConf(not_exist_i64, "/conf/not_exist_node", true, 100);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(100, not_exist_i64);

    ////////////////////////////////////////////////////////////////////////////////
    f64 not_exist_f64;

    ret = conf_center_->GetConf(not_exist_f64, "/conf/not_exist_node");
    EXPECT_TRUE(ret != 0);

    ret = conf_center_->GetConf(not_exist_f64, "/conf/not_exist_node", true, 100.0);
    EXPECT_EQ(0, ret);
    EXPECT_DOUBLE_EQ(100.0, not_exist_f64);

    ////////////////////////////////////////////////////////////////////////////////
    char* not_exist_str = NULL;

    ret = conf_center_->GetConf(&not_exist_str, "/conf/not_exist_node");
    EXPECT_TRUE(ret != 0);

    ret = conf_center_->GetConf(&not_exist_str, "/conf/not_exist_node", true, "xx");
    EXPECT_EQ(0, ret);
    EXPECT_STREQ("xx", not_exist_str);
    conf_center_->ReleaseConf(&not_exist_str);

    ////////////////////////////////////////////////////////////////////////////////
    // 不存在的属性
    ////////////////////////////////////////////////////////////////////////////////
    ret = conf_center_->GetConf(not_exist_i64, "/conf/db/@not_exist_attrib");
    EXPECT_TRUE(ret != 0);

    ret = conf_center_->GetConf(not_exist_i64, "/conf/db/@not_exist_attrib", true, 100);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(100, not_exist_i64);

    ////////////////////////////////////////////////////////////////////////////////
    ret = conf_center_->GetConf(not_exist_f64, "/conf/db/@not_exist_attrib");
    EXPECT_TRUE(ret != 0);

    ret = conf_center_->GetConf(not_exist_f64, "/conf/db/@not_exist_attrib", true, 100.0);
    EXPECT_EQ(0, ret);
    EXPECT_DOUBLE_EQ(100.0, not_exist_f64);

    ////////////////////////////////////////////////////////////////////////////////
    ret = conf_center_->GetConf(&not_exist_str, "/conf/db/@not_exist_attrib");
    EXPECT_TRUE(ret != 0);

    ret = conf_center_->GetConf(&not_exist_str, "/conf/db/@not_exist_attrib", true, "xx");
    EXPECT_EQ(0, ret);
    EXPECT_STREQ("xx", not_exist_str);
    conf_center_->ReleaseConf(&not_exist_str);
}

void ConfCenterTest::Test002()
{
    ////////////////////////////////////////////////////////////////////////////////
    // 取组节点
    ////////////////////////////////////////////////////////////////////////////////
    i32* port_group = NULL;
    int n;

    int ret = conf_center_->GetConf(&port_group, n, "/conf/port_list/port");
    EXPECT_EQ(0, ret);
    EXPECT_EQ(3, n);

    for (int i = 0; i < n; ++i)
    {
        std::cout << "port: " << port_group[i] << std::endl;
    }

    conf_center_->ReleaseConf(&port_group);

    ret = conf_center_->GetConf(&port_group, n, "/conf/port_list/port");
    EXPECT_EQ(0, ret);
    EXPECT_EQ(3, n);

    for (int i = 0; i < n; ++i)
    {
        std::cout << "port: " << port_group[i] << std::endl;
    }

    conf_center_->ReleaseConf(&port_group);

    ////////////////////////////////////////////////////////////////////////////////
    f32* coef_group = NULL;

    ret = conf_center_->GetConf(&coef_group, n, "/conf/coef_list/coef");
    EXPECT_EQ(0, ret);
    EXPECT_EQ(3, n);

    for (int i = 0; i < n; ++i)
    {
        std::cout << "coef: " << coef_group[i] << std::endl;
    }

    conf_center_->ReleaseConf(&coef_group);

    ret = conf_center_->GetConf(&coef_group, n, "/conf/coef_list/coef");
    EXPECT_EQ(0, ret);
    EXPECT_EQ(3, n);

    for (int i = 0; i < n; ++i)
    {
        std::cout << "coef: " << coef_group[i] << std::endl;
    }

    conf_center_->ReleaseConf(&coef_group);

    ////////////////////////////////////////////////////////////////////////////////
    char** ip_group = NULL;

    ret = conf_center_->GetConf(&ip_group, n, "/conf/ip_list/ip", false);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(3, n);

    for (int i = 0; i < n; ++i)
    {
        std::cout << "ip: " << ip_group[i] << std::endl;
    }

    conf_center_->ReleaseConf(&ip_group, n);

    ret = conf_center_->GetConf(&ip_group, n, "/conf/ip_list/ip");
    EXPECT_EQ(0, ret);
    EXPECT_EQ(3, n);

    for (int i = 0; i < n; ++i)
    {
        std::cout << "ip: " << ip_group[i] << std::endl;
    }

    conf_center_->ReleaseConf(&ip_group, n);

    ////////////////////////////////////////////////////////////////////////////////
    // 取组属性
    ////////////////////////////////////////////////////////////////////////////////
    i64* nthreads_group = NULL;

    ret = conf_center_->GetConf(&nthreads_group, n, "/conf/db_list/db/@threads");
    EXPECT_EQ(0, ret);
    EXPECT_EQ(3, n);

    for (int i = 0; i < n; ++i)
    {
        std::cout << "nthreads: " << nthreads_group[i] << std::endl;
    }

    conf_center_->ReleaseConf(&nthreads_group);

    ret = conf_center_->GetConf(&nthreads_group, n, "/conf/db_list/db/@threads");
    EXPECT_EQ(0, ret);
    EXPECT_EQ(3, n);

    for (int i = 0; i < n; ++i)
    {
        std::cout << "nthreads: " << nthreads_group[i] << std::endl;
    }

    conf_center_->ReleaseConf(&nthreads_group);

    ////////////////////////////////////////////////////////////////////////////////
    f64* db_coef_group = NULL;

    ret = conf_center_->GetConf(&db_coef_group, n, "/conf/db_list/db/@coef");
    EXPECT_EQ(0, ret);
    EXPECT_EQ(3, n);

    for (int i = 0; i < n; ++i)
    {
        std::cout << "coef: " << db_coef_group[i] << std::endl;
    }

    conf_center_->ReleaseConf(&db_coef_group);

    ret = conf_center_->GetConf(&db_coef_group, n, "/conf/db_list/db/@coef");
    EXPECT_EQ(0, ret);
    EXPECT_EQ(3, n);

    for (int i = 0; i < n; ++i)
    {
        std::cout << "coef: " << db_coef_group[i] << std::endl;
    }

    conf_center_->ReleaseConf(&db_coef_group);

    ////////////////////////////////////////////////////////////////////////////////
    char** db_name_group = NULL;

    ret = conf_center_->GetConf(&db_name_group, n, "/conf/db_list/db/@name");
    EXPECT_EQ(0, ret);
    EXPECT_EQ(3, n);

    for (int i = 0; i < n; ++i)
    {
        std::cout << "db_name: " << db_name_group[0] << std::endl;
    }

    conf_center_->ReleaseConf(&db_name_group, n);

    ret = conf_center_->GetConf(&db_name_group, n, "/conf/db_list/db/@name");
    EXPECT_EQ(0, ret);
    EXPECT_EQ(3, n);

    for (int i = 0; i < n; ++i)
    {
        std::cout << "db_name: " << db_name_group[0] << std::endl;
    }

    conf_center_->ReleaseConf(&db_name_group, n);
}

/**
 * @brief 配置文件不存在或者配置文件格式有错
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void ConfCenterTest::Test003()
{

}

/**
 * @brief 配置文件中有xml格式必须转义的字符
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void ConfCenterTest::Test004()
{

}

/**
 * @brief 字符串类型，找不到配置，使用默认值""
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void ConfCenterTest::Test005()
{
    char* myconf = NULL;
    int ret = conf_center_->GetConf(&myconf, "/conf/myconf", true, "");
    ASSERT_EQ(0, ret);

    std::cout << myconf << std::endl;
    conf_center_->ReleaseConf(&myconf);
}

/**
 * @brief 各种类型的group测试，找不到配置，使用默认值。字符串使用默认值""
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void ConfCenterTest::Test006()
{
    {
        i32* myconf_i32 = NULL;
        int n = 0;
        int ret = conf_center_->GetConf(&myconf_i32, n, "/conf/myconf_i32", true, 0);
        ASSERT_EQ(0, ret);
        ASSERT_EQ(1, n);
        ASSERT_EQ(0, myconf_i32[0]);
        conf_center_->ReleaseConf(&myconf_i32);
    }

    {
        i64* myconf_i64 = NULL;
        int n = 0;
        int ret = conf_center_->GetConf(&myconf_i64, n, "/conf/myconf_i64", true, 0L);
        ASSERT_EQ(0, ret);
        ASSERT_EQ(1, n);
        ASSERT_EQ(0L, myconf_i64[0]);
        conf_center_->ReleaseConf(&myconf_i64);
    }

    {
        f32* myconf_f32 = NULL;
        int n = 0;
        int ret = conf_center_->GetConf(&myconf_f32, n, "/conf/myconf_f32", true, 0.0f);
        ASSERT_EQ(0, ret);
        ASSERT_EQ(1, n);
        ASSERT_FLOAT_EQ(0.0f, myconf_f32[0]);
        conf_center_->ReleaseConf(&myconf_f32);
    }

    {
        f64* myconf_f64 = NULL;
        int n = 0;
        int ret = conf_center_->GetConf(&myconf_f64, n, "/conf/myconf_f64", true, 0.0);
        ASSERT_EQ(0, ret);
        ASSERT_EQ(1, n);
        ASSERT_FLOAT_EQ(0.0, myconf_f64[0]);
        conf_center_->ReleaseConf(&myconf_f64);
    }

    {
        char** myconf_str = NULL;
        int n = 0;
        int ret = conf_center_->GetConf(&myconf_str, n, "/conf/myconf_str", true, "");
        ASSERT_EQ(0, ret);
        ASSERT_EQ(1, n);
        ASSERT_STREQ("", myconf_str[0]);
        conf_center_->ReleaseConf(&myconf_str, n);
    }
}

ADD_TEST_F(ConfCenterTest, Test001);
ADD_TEST_F(ConfCenterTest, Test002);
ADD_TEST_F(ConfCenterTest, Test003);
ADD_TEST_F(ConfCenterTest, Test004);
ADD_TEST_F(ConfCenterTest, Test005);
ADD_TEST_F(ConfCenterTest, Test006);
