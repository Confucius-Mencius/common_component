#include <pthread.h>
#include <map>
#include <iostream>
#include "test_util.h"
#include "zookeeper.h"
#include "zookeeper_log.h"
#include "boost/uuid/detail/sha1.hpp"
#include "boost/archive/iterators/base64_from_binary.hpp"
#include "boost/archive/iterators/transform_width.hpp"

//pthread_mutex_lock(&zk_mutex);
//pthread_mutex_unlock(&zk_mutex);

static FILE* zk_log_stream = NULL;
static zhandle_t* zk_handle = NULL;
static int zk_reconnect_flag = 0;
static int zk_nzones = 0;
static int zk_zone_n = 0;
typedef std::map<std::string, std::string> ConfItemMap;
ConfItemMap conf_item_map;
static pthread_mutex_t zk_mutex = PTHREAD_MUTEX_INITIALIZER;

static const char* state2string(int state)
{
    if (state == 0)
    {
        return "CLOSED_STATE";
    }
    if (state == ZOO_CONNECTING_STATE)
    {
        return "CONNECTING_STATE";
    }
    if (state == ZOO_ASSOCIATING_STATE)
    {
        return "ASSOCIATING_STATE";
    }
    if (state == ZOO_CONNECTED_STATE)
    {
        return "CONNECTED_STATE";
    }
    if (state == ZOO_EXPIRED_SESSION_STATE)
    {
        return "EXPIRED_SESSION_STATE";
    }
    if (state == ZOO_AUTH_FAILED_STATE)
    {
        return "AUTH_FAILED_STATE";
    }

    return "INVALID_STATE";
}

static const char* type2string(int type)
{
    if (type == ZOO_CREATED_EVENT)
    {
        return "CREATED_EVENT";
    }
    if (type == ZOO_DELETED_EVENT)
    {
        return "DELETED_EVENT";
    }
    if (type == ZOO_CHANGED_EVENT)
    {
        return "CHANGED_EVENT";
    }
    if (type == ZOO_CHILD_EVENT)
    {
        return "CHILD_EVENT";
    }
    if (type == ZOO_SESSION_EVENT)
    {
        return "SESSION_EVENT";
    }
    if (type == ZOO_NOTWATCHING_EVENT)
    {
        return "NOTWATCHING_EVENT";
    }

    return "UNKNOWN_EVENT_TYPE";
}

static void watcher(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx);

static void zk_connect()
{
    LOG_INFO(("thread id: %ld, TryConnectZk", pthread_self()));

    const char* host = "127.0.0.1:2181";
    int timeout = 30000; // 4s - 40s

    LOG_INFO(("zookeeper_init begin"));

    zk_handle = zookeeper_init(host, watcher, timeout, NULL, NULL, 0);
    if (NULL == zk_handle)
    {
        LOG_ERROR(("Error when connecting to zookeeper servers..."));
        FAIL();
    }

    LOG_INFO(("zookeeper_init end"));
}

static void data_completion(int rc, const char* value, int value_len, const struct Stat* stat, const void* data)
{
    LOG_INFO(("thread id: %ld, DataCompletion, rc: %d", pthread_self(), rc));

    if (ZOK == rc && value != NULL)
    {
        LOG_INFO(("value: %s, value len: %d", value, value_len));
        std::string v;
        v.assign(value, value_len);

        pthread_mutex_lock(&zk_mutex);
        conf_item_map.insert(ConfItemMap::value_type((char*) data, v));
        pthread_mutex_unlock(&zk_mutex);
    }

    free((void*) data);
    ++zk_zone_n;

    if (zk_zone_n == zk_nzones)
    {
        // 所有的异步请求都回来了
        pthread_mutex_lock(&zk_mutex);
        for (ConfItemMap::const_iterator it = conf_item_map.begin(); it != conf_item_map.end(); ++it)
        {
            LOG_INFO(("zone: %s, value: %s", it->first.c_str(), it->second.c_str()));
        }
        pthread_mutex_unlock(&zk_mutex);
    }
}

static void strings_completion(int rc, const struct String_vector* strings, const void* data)
{
    LOG_INFO(("thread id: %ld, StringsCompletion, rc: %d", pthread_self(), rc));

    if (ZOK == rc)
    {
        zk_nzones = strings->count;
        zk_zone_n = 0;
        conf_item_map.clear();

        zhandle_t* zh = (zhandle_t*) data;

        for (int i = 0; i < strings->count; ++i)
        {
            LOG_INFO(("%s", strings->data[i]));

            char* node_name = strdup(strings->data[i]);
            char node_path[256] = "";
            snprintf(node_path, sizeof(node_path), "/easydown/zone/%s", node_name);

            int ret = zoo_aget(zh, node_path, 0, data_completion, node_name);
            if (ret != ZOK)
            {
                LOG_ERROR(("error %s", zerror(ret)));
                return;
            }
        }
    }
}

static int zk_get_conf(zhandle_t* zh)
{
    LOG_INFO(("thread id: %ld, zk_get_conf", pthread_self()));

    int ret = zoo_aget_children(zh, "/easydown/zone", 1, strings_completion, zh);
    if (ZOK != ret)
    {
        LOG_ERROR(("error %s", zerror(ret))); // 通过zerror获取错误描述字符串
        return -1;
    }

    return 0;
}

static void watcher(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx)
{
    LOG_INFO(("thread id: %ld, watcher, type: %d, state: %d", pthread_self(), type, state));

    if (type == ZOO_SESSION_EVENT)
    {
        if (state == ZOO_CONNECTED_STATE)
        {
            LOG_INFO(("connected zookeeper"));
            //第一次正常连接和超时重连都会触发该状态，所以要判断是否是超时引起的该状态
            //如果是会话超时引起的，需要重新设置观察器
            //对之前注册的每个路径，都需要显式的触发一次
            //（例如原来是调用的get_children监视，那么现在需要调用get_children获取一次）
            //防止因为和zk的会话超时，导致这段时间内的节点变化监视丢失
            if (zk_reconnect_flag)
            {
                zk_reconnect_flag = 0;
                //重新设置对路径的观察事件
            }

            zk_get_conf(zh);
        }
        else if (state == ZOO_AUTH_FAILED_STATE)
        {
            LOG_ERROR(("Authentication failure. Shutting down..."));
            zookeeper_close(zh);
        }
        else if (state == ZOO_EXPIRED_SESSION_STATE)
        {
            LOG_ERROR(("Session expired. Shutting down..."));
            //超时会话过期，设置重连标记位
            zk_reconnect_flag = 1;
            //关闭原来的zookeeper handle，并且用zookeeper_init尝试重连
            zookeeper_close(zh);
            zk_connect();
        }
    }
    else
    {
        //判断事件和路径，分发给不同的调用逻辑
        if (type == ZOO_CHILD_EVENT)
        {
            zk_get_conf(zh);
        }
    }
}

static void Test001()
{
    LOG_INFO(("thread id: %ld, Test001", pthread_self()));

    zk_log_stream = fopen("/tmp/zk.log", "a+");
    if (NULL == zk_log_stream)
    {
        FAIL();
    }

    zoo_set_log_stream(zk_log_stream);
    zoo_set_debug_level(ZOO_LOG_LEVEL_DEBUG);

    zk_connect();

    // Wait for asynchronous zookeeper call done.
    LOG_INFO(("sleep"));
    sleep(30);

    if (zk_log_stream != NULL)
    {
        fclose(zk_log_stream);
        zk_log_stream = NULL;
    }

    if (zk_handle != NULL)
    {
        zookeeper_close(zk_handle);
        zk_handle = NULL;
    }
}

//static void ensureConnected()
//{
//    pthread_mutex_lock(&lock);
//    while (zoo_state(zh)!=ZOO_CONNECTED_STATE)
//    {
//        pthread_cond_wait(&cond, &lock);
//    }
//    pthread_mutex_unlock(&lock);
//}

// 建好结点
static void Test002()
{
    const char* host = "10.0.0.192:2181";
    int timeout = 30000; // 4s - 40s

    LOG_INFO(("zookeeper_init begin"));

    zhandle_t* zk_handle = zookeeper_init(host, NULL, timeout, NULL, NULL, 0);
    if (NULL == zk_handle)
    {
        LOG_ERROR(("Error when connecting to zookeeper servers..."));
        FAIL();
    }

//    int ret = zoo_create(zk_handle, "/galileo", NULL, 0, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0);
//    if (ret != ZOK)
//    {
//        LOG_ERROR(("ret: %d", ret));
//        FAIL();
//    }

//    int ret = zoo_create(zk_handle, "/galileo/topic", NULL, 0, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0);
//    if (ret != ZOK)
//    {
//        LOG_ERROR(("ret: %d", ret));
//        FAIL();
//    }

    int ret = zoo_create(zk_handle, "/galileo/topic/star", NULL, 0, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0);
    if (ret != ZOK)
    {
        LOG_ERROR(("ret: %d", ret));
        FAIL();
    }
//
//    for (int i = 0; i < 10; ++i)
//    {
//        char node_path[256] = "";
//        snprintf(node_path, sizeof(node_path), "/easydown/zone/%d", i);
//
//        char node_data[64] = "";
//        snprintf(node_data, sizeof(node_data), "data%d", i);
//
//        ret = zoo_create(zk_handle, node_path, node_data, strlen(node_data), &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0);
//        if (ret != ZOK)
//        {
//            LOG_ERROR(("ret: %d", ret));
//            FAIL();
//        }
//    }

    if (zk_handle != NULL)
    {
        zookeeper_close(zk_handle);
        zk_handle = NULL;
    }
}

static void Test003()
{
    const char* host = "192.168.2.2:2181"; // 测试环境zk
//    const char* host = "zk.moon.com:8081"; // 线上环境
    int timeout = 30000; // 4s - 40s

    LOG_INFO(("zookeeper_init begin"));

    zhandle_t* zk_handle = zookeeper_init(host, NULL, timeout, NULL, NULL, 0);
    if (NULL == zk_handle)
    {
        LOG_ERROR(("Error when connecting to zookeeper servers..."));
        FAIL();
    }

    sleep(5);

    int ret = zoo_exists(zk_handle, "/easydown/zone", 0, NULL); // zoo_exists检查节点是否存在，调用之前不需要zoo_add_auth设置访问权限
    std::cout << "1----------- ret: " << ret << std::endl;

    ret = zoo_exists(zk_handle, "/easydown/file", 0, NULL);
    std::cout << "2----------- ret: " << ret << std::endl;

    char buf[1024] = "";
    int buf_len = sizeof(buf_len);

    ret = zoo_add_auth(zk_handle, "digest", "easydown:UjF5QjRfX3DdYm3Z", strlen("easydown:UjF5QjRfX3DdYm3Z"), NULL,
                       0);
    if (ret != ZOK)
    {
        FAIL() << ret;
    }

    ret = zoo_get(zk_handle, "/easydown/zone", 0, buf, &buf_len, NULL); // 当节点有权限时，调用之前需要zoo_add_auth设置访问权限
    std::cout << "3----------- ret: " << ret << std::endl;

    {
        char node_path[256] = "";
        snprintf(node_path, sizeof(node_path), "/easydown/zone/huang");

        char node_data[64] = "";
        snprintf(node_data, sizeof(node_data), "*.moon.com");

        ret = zoo_create(zk_handle, node_path, node_data, strlen(node_data), &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0);
        if (ret != ZOK)
        {
            LOG_ERROR(("ret: %d", ret));
            FAIL();
        }
    }

    sleep(4);;
    ret = zoo_get(zk_handle, "/easydown/zone/huang", 0, buf, &buf_len,
                  NULL); // 当节点没有权限时，用设置了访问权限的zk_handle去get节点数据，是ok的
    std::cout << "4----------- ret: " << ret << std::endl;

    return;

//    int ret = zoo_delete(zk_handle, "/easydown/zone", -1);
//    std::cout << "1 ----------- ret: " << ret << std::endl;

//    int ret = zoo_add_auth(zk_handle, "digest", "easydown:UjF5QjRfX3DdYm3Z", strlen("easydown:UjF5QjRfX3DdYm3Z"), NULL,
//                           0);
//    if (ret != ZOK)
//    {
//        FAIL() << ret;
//    }
//
//    ret = zoo_exists(zk_handle, "/easydown/zone", 0, NULL);
//    std::cout << "----------- ret: " << ret << std::endl;


    char scheme[10] = "digest";

    // user:base64encode(sha1(user:passwd))
    const char user_pass[] = "easydown:UjF5QjRfX3DdYm3Z";
    boost::uuids::detail::sha1 sha;
    sha.process_bytes(user_pass, strlen(user_pass));
    uint32_t digest[5] = {0};
    sha.get_digest(digest);

    std::string str_sha1;
    for (int i = 0; i < 5; ++i)
    {
        str_sha1.append(1, static_cast<char>((digest[i] >> 24) & 0xFF));
        str_sha1.append(1, static_cast<char>((digest[i] >> 16) & 0xFF));
        str_sha1.append(1, static_cast<char>((digest[i] >> 8) & 0xFF));
        str_sha1.append(1, static_cast<char>((digest[i]) & 0xFF));
    }

    typedef boost::archive::iterators::base64_from_binary<boost::archive::iterators::transform_width<std::string::const_iterator,
        6,
        8> > Base64EncodeIterator;
    std::stringstream result;
    std::copy(Base64EncodeIterator(str_sha1.begin()), Base64EncodeIterator(str_sha1.end()),
              std::ostream_iterator<char>(result));
    size_t equal_count = (3 - str_sha1.length() % 3) % 3;
    for (size_t i = 0; i < equal_count; i++)
    {
        result.put('=');
    }

    char usr_pa[100] = "";
    snprintf(usr_pa, sizeof(usr_pa), "%s:%s", "easydown", result.str().c_str());

    struct ACL acl;
    acl.perms = ZOO_PERM_ALL;
    acl.id.scheme = scheme;
    acl.id.id = usr_pa;

    struct ACL_vector aclv;
    aclv.count = 1;
    aclv.data = &acl;

    ret = zoo_create(zk_handle, "/easydown/zone", NULL, 0, &aclv, 0, NULL, 0);
    if (ret != ZOK)
    {
        LOG_ERROR(("ret: %d", ret));
        FAIL();
    }

    ret = zoo_create(zk_handle, "/easydown/file", NULL, 0, &aclv, 0, NULL, 0);
    if (ret != ZOK)
    {
        LOG_ERROR(("ret: %d", ret));
        FAIL();
    }

    sleep(5);

    ret = zoo_exists(zk_handle, "/easydown/zone", 0, NULL);
    std::cout << "----------- ret: " << ret << std::endl;

    ret = zoo_exists(zk_handle, "/easydown/file", 0, NULL);
    std::cout << "----------- ret: " << ret << std::endl;

    if (zk_handle != NULL)
    {
        zookeeper_close(zk_handle);
        zk_handle = NULL;
    }
}

static void Test004_data_completion(int rc, const char* value, int value_len, const struct Stat* stat, const void* data)
{
    LOG_INFO(("Test004_data_completion called, pid: %d, thread id: %ld, rc: %d", getpid(), pthread_self(), rc));
    if (ZOK == rc && value != NULL)
    {
        LOG_INFO(("value: %s, value len: %d, data: %s", value, value_len, data));
    }
}

static void Test004_strings_completion(int rc, const struct String_vector* strings, const void* data)
{
    LOG_INFO(("Test004_strings_completion called, pid: %d, thread id: %ld, rc: %d", getpid(), pthread_self(), rc));

    if (ZOK == rc)
    {
        zhandle_t* zh = (zhandle_t*) data;
        int i;

        for (i = 0; i < strings->count; ++i)
        {
            LOG_INFO(("%s", strings->data[i]));

            char* name = strdup(strings->data[i]);
            char path[256] = "";
            snprintf(path, sizeof(path), "/easydown/%s/%s", "zone", name);

            int ret = zoo_aget(zh, path, 0, Test004_data_completion, name);
            if (ret != ZOK)
            {
                LOG_ERROR(("zoo_aget failed, err msg: %s", zerror(ret)));
                return;
            }
        }
    }
}

static void Test004()
{
//    const char* host = "192.168.2.2:2181"; // 测试环境zk
    const char* host = "zk.moon.com:8081"; // 线上环境

    int timeout = 30000; // 4s - 40s

    LOG_INFO(("zookeeper_init begin"));

    zhandle_t* zk_handle = zookeeper_init(host, NULL, timeout, NULL, NULL, 0);
    if (NULL == zk_handle)
    {
        LOG_ERROR(("Error when connecting to zookeeper servers..."));
        FAIL();
    }

    int ret = zoo_aget_children(zk_handle, "/easydown/zone", 1, Test004_strings_completion, zk_handle);
    if (ZOK != ret)
    {
        LOG_ERROR(("zoo_aget_children failed, err msg: %s", zerror(ret)));
        FAIL();
    }

    ret = zoo_aget_children(zk_handle, "/easydown/file", 1, Test004_strings_completion, zk_handle);
    if (ZOK != ret)
    {
        LOG_ERROR(("zoo_aget_children failed, err msg: %s", zerror(ret)));
        FAIL();
    }

    sleep(10);

    if (zk_handle != NULL)
    {
        zookeeper_close(zk_handle);
        zk_handle = NULL;
    }
}

static void Test005()
{
    const char* host = "192.168.2.2:2181"; // 测试环境zk
    int timeout = 30000; // 4s - 40s

    LOG_INFO(("zookeeper_init begin"));

    zhandle_t* zk_handle = zookeeper_init(host, NULL, timeout, NULL, NULL, 0);
    if (NULL == zk_handle)
    {
        LOG_ERROR(("Error when connecting to zookeeper servers..."));
        FAIL();
    }

    {
        char node_path[256] = "";
        snprintf(node_path, sizeof(node_path), "/easydown/file/blue.cjstyles");

        char node_data[64] = "";
        snprintf(node_data, sizeof(node_data), "*.moon.com");

        int ret = zoo_create(zk_handle, node_path, node_data, strlen(node_data), &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0);
        if (ret != ZOK)
        {
            LOG_ERROR(("ret: %d", ret));
            FAIL();
        }
    }

    if (zk_handle != NULL)
    {
        zookeeper_close(zk_handle);
        zk_handle = NULL;
    }
}

static void Test006()
{
    const char* host = "10.0.0.192:2181";
    int timeout = 30000; // 4s - 40s

    LOG_INFO(("zookeeper_init begin"));

    zhandle_t* zk_handle = zookeeper_init(host, NULL, timeout, NULL, NULL, 0);
    if (NULL == zk_handle)
    {
        LOG_ERROR(("Error when connecting to zookeeper servers..."));
        FAIL();
    }

    {
        char node_path[256] = "";
        snprintf(node_path, sizeof(node_path), "/galileo/topic/abc");

        char node_data[64] = "";
        snprintf(node_data, sizeof(node_data), "%ld", time(NULL));

        int ret = zoo_set(zk_handle, node_path, node_data, strlen(node_data), -1);
        if (ret != ZOK)
        {
            LOG_ERROR(("ret: %d", ret));
            FAIL();
        }
    }

    sleep(3);

    if (zk_handle != NULL)
    {
        zookeeper_close(zk_handle);
        zk_handle = NULL;
    }
}

static void Test007()
{
    const char* host = "192.168.2.16:2181"; // 测试环境zk
//    const char* host = "zk.moon.com:8081"; // 线上环境
    int timeout = 30000; // 4s - 40s

    LOG_INFO(("zookeeper_init begin"));

    zhandle_t* zk_handle = zookeeper_init(host, NULL, timeout, NULL, NULL, 0);
    if (NULL == zk_handle)
    {
        LOG_ERROR(("Error when connecting to zookeeper servers..."));
        FAIL();
    }

    int ret = zoo_exists(zk_handle, "/luna_abc", 0, NULL); // zoo_exists检查节点是否存在，调用之前不需要zoo_add_auth设置访问权限
    std::cout << "1----------- ret: " << ret << std::endl;

    ret = zoo_add_auth(zk_handle, "digest", "easydown:UjF5QjRfX3DdYm3Z", strlen("easydown:UjF5QjRfX3DdYm3Z"), NULL,
                       0);
    if (ret != ZOK)
    {
        FAIL() << ret;
    }

//    char scheme[10] = "digest";
//
//    // user:base64encode(sha1(user:passwd))
//    const char user_pass[] = "easydown:UjF5QjRfX3DdYm3Z";
//    boost::uuids::detail::sha1 sha;
//    sha.process_bytes(user_pass, strlen(user_pass));
//    uint32_t digest[5] = {0};
//    sha.get_digest(digest);
//
//    std::string str_sha1;
//    for (int i = 0; i < 5; ++i)
//    {
//        str_sha1.append(1, static_cast<char>((digest[i] >> 24) & 0xFF));
//        str_sha1.append(1, static_cast<char>((digest[i] >> 16) & 0xFF));
//        str_sha1.append(1, static_cast<char>((digest[i] >> 8) & 0xFF));
//        str_sha1.append(1, static_cast<char>((digest[i]) & 0xFF));
//    }
//
//    typedef boost::archive::iterators::base64_from_binary<boost::archive::iterators::transform_width<std::string::const_iterator,
//        6,
//        8> > Base64EncodeIterator;
//    std::stringstream result;
//    std::copy(Base64EncodeIterator(str_sha1.begin()), Base64EncodeIterator(str_sha1.end()),
//              std::ostream_iterator<char>(result));
//    size_t equal_count = (3 - str_sha1.length() % 3) % 3;
//    for (size_t i = 0; i < equal_count; i++)
//    {
//        result.put('=');
//    }
//
//    std::cout << result.str() << std::endl;
//
//    char usr_pa[100] = "";
//    snprintf(usr_pa, sizeof(usr_pa), "%s:%s", "easydown", result.str().c_str());
//
//    struct ACL acl;
//    acl.perms = ZOO_PERM_ALL;
//    acl.id.scheme = scheme;
//    acl.id.id = usr_pa;
//
//    struct ACL_vector aclv;
//    aclv.count = 1;
//    aclv.data = &acl;
//
//    ret = zoo_create(zk_handle, "/luna_abc", NULL, 0, &aclv, 0, NULL, 0);
//    if (ret != ZOK)
//    {
//        LOG_ERROR(("ret: %d", ret));
//        FAIL();
//    }
//
    char buf[1024] = "";
    int buf_len = sizeof(buf_len);
//
//    ret = zoo_get(zk_handle, "/luna_abc", 0, buf, &buf_len, NULL); // 当节点有权限时，调用之前需要zoo_add_auth设置访问权限
//    std::cout << "3----------- ret: " << ret << std::endl;


    ret = zoo_create(zk_handle, "/luna_abc/status", NULL, 0, &ZOO_CREATOR_ALL_ACL, 0, NULL, 0);
    if (ret != ZOK)
    {
        LOG_ERROR(("ret: %d", ret));
        FAIL();
    }


    ret = zoo_get(zk_handle, "/luna_abc/status", 0, buf, &buf_len, NULL); // 当节点有权限时，调用之前需要zoo_add_auth设置访问权限
    std::cout << "3----------- ret: " << ret << std::endl;

}

ADD_TEST(ZkRealTest, Test001);
//ADD_TEST(ZkRealTest, Test002);
//ADD_TEST(ZkRealTest, Test003);
//ADD_TEST(ZkRealTest, Test004);
//ADD_TEST(ZkRealTest, Test005);
//ADD_TEST(ZkRealTest, Test006);
ADD_TEST(ZkRealTest, Test007);
