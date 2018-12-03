#include "zk_util_test.h"
#include "zk_util.h"

void OnConnectedEvent(const char* path, zk::EventType event_type, const void* args)
{
    std::cout << "xx" << std::endl;

//    LOG_TRACE("path: " << (path != NULL ? path : "") << ", event_type: " << event_type);
//    ZkClient* zk_client = (ZkClient*) args;
//    Logic* global_logic = (Logic*) zk_client->logic_ctx_->global_logic;
//
//    char node_path[64]  ="";
//
//    for (int i = proto::CONF_ID_MIN; i < proto::CONF_ID_MAX; ++i)
//    {
//        StrPrintf(node_path, sizeof(node_path), "%s/%d", global_logic->GetConfMgr()->GetZkConfNode().c_str(), i);
//        zk::GetNode(node_path, 1, ZkClient::OnGetNodeCompletion, zk_client);
//    }
}

void OnChangedEvent(const char* path, zk::EventType event_type, const void* args)
{
//    LOG_TRACE("path: " << (path != NULL ? path : "") << ", event_type: " << event_type);
//    ZkClient* zk_client = (ZkClient*) args;
//    Logic* global_logic = (Logic*) zk_client->logic_ctx_->global_logic;
//
//    if (path != NULL)
//    {
//        const char* p = strstr(path, global_logic->GetConfMgr()->GetZkConfNode().c_str());
//        if (p != path)
//        {
//            return;
//        }
//
//        zk::GetNode(path, 1, ZkClient::OnGetNodeCompletion, zk_client);
//    }
}

void OnGetNodeCompletion(const char* path, const char* node_name, const zk::NodeCtx* node_ctx, const void* args)
{
//    LOG_WARN("path: " << (path != NULL ? path : "") << ", node name: " << node_name);
//    ZkClient* zk_client = (ZkClient*) args;
//
//    if (path != NULL)
//    {
//        zk_client->cache_mgr_->CheckVersion(atoi(node_name), node_ctx);
//    }
}

ZkUtilTest::ZkUtilTest()
{

}

ZkUtilTest::~ZkUtilTest()
{

}

void ZkUtilTest::SetUp()
{
    zk::Conf zk_conf;
    zk_conf.zk_host = "zk.moon.com:8081";
    zk_conf.zk_timeout = 10;
    zk_conf.zk_log_level = 4;
    zk_conf.zk_log_file_path = "/tmp/zk.log";
    zk_conf.zk_user_passwd = "";
    zk_conf.event_conf.connected_event.func = OnConnectedEvent;
    zk_conf.event_conf.connected_event.args = this;
    zk_conf.event_conf.children_event.func = NULL;
    zk_conf.event_conf.children_event.args = this;
    zk_conf.event_conf.changed_event.func = OnChangedEvent;
    zk_conf.event_conf.changed_event.args = this;

    if (zk::Initialize(&zk_conf) != 0)
    {
        std::cout << "failed to initialize zookeeper";
    }
}

void ZkUtilTest::TearDown()
{
    zk::Finalize();
}

void ZkUtilTest::Test001()
{

}

ADD_TEST_F(ZkUtilTest, Test001);
