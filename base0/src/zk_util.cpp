#include "zk_util.h"
#include <unistd.h>
#include <string.h>
#include "str_util.h"
#include "file_util.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include "zookeeper_log.h"

#ifdef __cplusplus
}
#endif // __cplusplus

#ifndef ZOOKEEPER_LOG_ERROR
#define ZOOKEEPER_LOG_ERROR(format, ...) LOG_ERROR((format, ##__VA_ARGS__))
#endif // ZOOKEEPER_LOG_ERROR

#ifndef ZOOKEEPER_LOG_INFO
#define ZOOKEEPER_LOG_INFO(format, ...) LOG_INFO((format, ##__VA_ARGS__))
#endif // ZOOKEEPER_LOG_INFO

namespace zk
{
struct Ctx
{
    Conf conf;
    FILE* zk_log_stream;
    zhandle_t* zk_handle;
    int zk_reconnect_flag;
    int zk_connected;

    Ctx() : conf()
    {
        zk_log_stream = NULL;
        zk_handle = NULL;
        zk_reconnect_flag = 0;
        zk_connected = 0;
    }

    ~Ctx()
    {
    }

    void Release()
    {
        if (zk_handle != NULL)
        {
            zookeeper_close(zk_handle);
            zk_handle = NULL;
        }

        if (zk_log_stream != NULL)
        {
            fclose(zk_log_stream);
            zk_log_stream = NULL;
        }
    }
};

static Ctx ctx;

////////////////////////////////////////////////////////////////////////////////
static const char* StateString(int state)
{
    if (state == 0)
    {
        return "CLOSED_STATE";
    }
    else if (state == ZOO_CONNECTING_STATE)
    {
        return "CONNECTING_STATE";
    }
    else if (state == ZOO_ASSOCIATING_STATE)
    {
        return "ASSOCIATING_STATE";
    }
    else if (state == ZOO_CONNECTED_STATE)
    {
        return "CONNECTED_STATE";
    }
    else if (state == ZOO_EXPIRED_SESSION_STATE)
    {
        return "EXPIRED_SESSION_STATE";
    }
    else if (state == ZOO_AUTH_FAILED_STATE)
    {
        return "AUTH_FAILED_STATE";
    }
    else
    {
        return "INVALID_STATE";
    }
}

static const char* EventTypeString(int type)
{
    if (type == ZOO_CREATED_EVENT)
    {
        return "CREATED_EVENT";
    }
    else if (type == ZOO_DELETED_EVENT)
    {
        return "DELETED_EVENT";
    }
    else if (type == ZOO_CHANGED_EVENT)
    {
        return "CHANGED_EVENT";
    }
    else if (type == ZOO_CHILD_EVENT)
    {
        return "CHILD_EVENT";
    }
    else if (type == ZOO_SESSION_EVENT)
    {
        return "SESSION_EVENT";
    }
    else if (type == ZOO_NOTWATCHING_EVENT)
    {
        return "NOTWATCHING_EVENT";
    }
    else
    {
        return "UNKNOWN_EVENT_TYPE";
    }
}

static int TryConnectZkeeper();

static void StatCompletion(int rc, const struct Stat* stat, const void* data)
{
    ZOOKEEPER_LOG_INFO("StatCompletion called, pid: %d, thread id: %ld, rc: %d", getpid(), pthread_self(), rc);
}

static void GlobalWatcher(zhandle_t* zh, int type, int state, const char* path, void* watcher_ctx)
{
    ZOOKEEPER_LOG_INFO("GlobalWatcher called, pid: %d, thread id: %ld, type: %d %s, state: %d %s",
                       getpid(), pthread_self(), type, EventTypeString(type), state, StateString(state));

    Ctx* ctx = (Ctx*) watcher_ctx;

    if (ZOO_SESSION_EVENT == type)
    {
        if (ZOO_CONNECTED_STATE == state)
        {
            ZOOKEEPER_LOG_INFO("connected to zookeeper server ok");
            ctx->zk_connected = 1;

            if (1 == ctx->zk_reconnect_flag)
            {
                ctx->zk_reconnect_flag = 0;
            }

            if (ctx->conf.zk_user_passwd.size() > 0)
            {
                zoo_add_auth(zh, "digest", ctx->conf.zk_user_passwd.data(), ctx->conf.zk_user_passwd.size(), NULL,
                             NULL);
            }

            if (ctx->conf.event_conf.connected_event.func != NULL)
            {
                ctx->conf.event_conf.connected_event.func("/", EVENT_TYPE_CONNECTED,
                                                          ctx->conf.event_conf.connected_event.args);
            }
        }
        else if (ZOO_AUTH_FAILED_STATE == state)
        {
            ZOOKEEPER_LOG_ERROR("zookeeper authentication failure");
            zookeeper_close(zh);
        }
        else if (ZOO_EXPIRED_SESSION_STATE == state)
        {
            ZOOKEEPER_LOG_ERROR("session expired, reconnecting...");
            zookeeper_close(zh);
            ctx->zk_reconnect_flag = 1;
            TryConnectZkeeper();
        }
    }
    else
    {
        if (ZOO_CHILD_EVENT == type)
        {
            if (ctx->conf.event_conf.children_event.func != NULL)
            {
                ctx->conf.event_conf.children_event.func(path, EVENT_TYPE_CHILDREN,
                                                         ctx->conf.event_conf.children_event.args);
            }
        }
        else if (ZOO_CHANGED_EVENT == type)
        {
            if (ctx->conf.event_conf.changed_event.func != NULL)
            {
                ctx->conf.event_conf.changed_event.func(path, EVENT_TYPE_CHANGED,
                                                        ctx->conf.event_conf.changed_event.args);
            }
        }

        // 再次关注该节点的事件
        int ret = zoo_aexists(zh, path, 1, StatCompletion, NULL);
        if (ret != ZOK)
        {
            ZOOKEEPER_LOG_ERROR("zoo_aexists failed， err msg: %s", zerror(ret));
        }
    }
}

static int TryConnectZkeeper()
{
    ZOOKEEPER_LOG_INFO("TryConnectZkeeper called, host: %s, timeout: %d, pid: %d, thread id: %ld",
                       ctx.conf.zk_host.c_str(),
                       ctx.conf.zk_timeout, getpid(), pthread_self());
    ctx.zk_connected = 0;

    ctx.zk_handle = zookeeper_init(ctx.conf.zk_host.c_str(), GlobalWatcher, ctx.conf.zk_timeout * 1000, NULL, &ctx, 0);
    if (NULL == ctx.zk_handle)
    {
        ZOOKEEPER_LOG_ERROR("zookeeper_init failed");
        return -1;
    }

    return 0;
}

int Initialize(const Conf* conf)
{
    ZOOKEEPER_LOG_INFO("Initialize called, pid: %d, thread id: %ld", getpid(), pthread_self());

    if (NULL == conf)
    {
        return -1;
    }

    ctx.conf = *conf;

    zoo_set_debug_level((ZooLogLevel) ctx.conf.zk_log_level);

    ctx.zk_log_stream = fopen(ctx.conf.zk_log_file_path.c_str(), "a+");
    if (NULL == ctx.zk_log_stream)
    {
        ZOOKEEPER_LOG_ERROR("fopen failed, file path: %s, err msg: %s", ctx.conf.zk_log_file_path.c_str(),
                            strerror(errno));
        return -1;
    }

    zoo_set_log_stream(ctx.zk_log_stream);

    ctx.zk_reconnect_flag = 0;

    if (TryConnectZkeeper() != 0)
    {
        return -1;
    }

    return 0;
}

void Finalize()
{
    ZOOKEEPER_LOG_INFO("Finalize called, pid: %d, thread id: %ld", getpid(), pthread_self());
    ctx.Release();
}

struct ChildrenCtx
{
    int children_count;
    int children_n;
    NodeCtxMap node_ctx_map;

    ChildrenCtx() : node_ctx_map()
    {
        children_count = 0;
        children_n = 0;
    }

    ~ChildrenCtx()
    {
    }
};

struct GetChildrenArgs
{
    zhandle_t* zh;
    std::string parent_path;
    ChildrenCtx children_ctx;
    int watch_child;
    GetChildrenCompletionFunc completion_func;
    const void* args;

    GetChildrenArgs() : parent_path(""), children_ctx()
    {
        zh = NULL;
        watch_child = 0;
        completion_func = NULL;
        args = NULL;
    }

    ~GetChildrenArgs()
    {
        Release();
    }

    static GetChildrenArgs* Create()
    {
        return new GetChildrenArgs();
    }

    void Release()
    {
        delete this;
    }
};

struct GetChildrenNodeArgs
{
    GetChildrenArgs* get_children_args;
    std::string name; // node_name
    std::string path; // node_path

    GetChildrenNodeArgs() : name(""), path("")
    {
        get_children_args = NULL;
    }

    ~GetChildrenNodeArgs()
    {
    }

    static GetChildrenNodeArgs* Create()
    {
        return new GetChildrenNodeArgs();
    }

    void Release()
    {
        delete this;
    }
};

static void ChildrenDataCompletion(int rc, const char* value, int value_len, const struct Stat* stat, const void* data)
{
//    struct Stat {
//        int64_t czxid; // 创建节点的事务的zxid
//        int64_t mzxid; // 对znode最近修改的zxid
//        int64_t ctime; // 以距离时间原点(epoch)的毫秒数表示的znode创建时间
//        int64_t mtime; // 以距离时间原点(epoch)的毫秒数表示的znode最近修改时间
//        int32_t version; // znode数据的修改次数
//        int32_t cversion; // znode子节点修改次数
//        int32_t aversion; // znode的ACL修改次数
//        int64_t ephemeralOwner; // 如果znode是临时节点，则指示节点所有者的会话ID；如果不是临时节点，则为零
//        int32_t dataLength; // znode数据长度
//        int32_t numChildren; // znode子节点个数
//        int64_t pzxid;
//    };

    ZOOKEEPER_LOG_INFO("ChildrenDataCompletion called, pid: %d, thread id: %ld, rc: %d", getpid(), pthread_self(), rc);
    GetChildrenNodeArgs* get_children_node_args = (GetChildrenNodeArgs*) data;
    ++get_children_node_args->get_children_args->children_ctx.children_n;

    if (ZOK == rc)
    {
        NodeCtx node_ctx;

        if (value != NULL && value_len > 0)
        {
            node_ctx.value.assign(value, value_len);
        }

        if (stat != NULL)
        {
            node_ctx.version = stat->version;
        }

        if (!get_children_node_args->get_children_args->children_ctx.node_ctx_map.insert(
            NodeCtxMap::value_type(get_children_node_args->name, node_ctx)).second)
        {
            ZOOKEEPER_LOG_ERROR("failed to insert to map");
        }
    }

    if (get_children_node_args->get_children_args->children_ctx.children_n ==
        get_children_node_args->get_children_args->children_ctx.children_count)
    {
        // 所有的异步请求都回来了
        if (get_children_node_args->get_children_args->completion_func != NULL)
        {
            get_children_node_args->get_children_args->completion_func(
                get_children_node_args->get_children_args->parent_path.c_str(),
                &get_children_node_args->get_children_args->children_ctx.node_ctx_map,
                get_children_node_args->get_children_args->args);
        }

        get_children_node_args->get_children_args->Release();
        get_children_node_args->get_children_args = NULL;
    }

    get_children_node_args->Release();
    get_children_node_args = NULL;
}

static void ChildrenStringsCompletion(int rc, const struct String_vector* strings, const void* data)
{
    ZOOKEEPER_LOG_INFO("ChildrenStringsCompletion called, pid: %d, thread id: %ld, rc: %d", getpid(), pthread_self(),
                       rc);
    GetChildrenArgs* get_children_args = (GetChildrenArgs*) data;

    if (ZOK == rc)
    {
        if (0 == strings->count)
        {
            if (get_children_args->completion_func != NULL)
            {
                get_children_args->completion_func(get_children_args->parent_path.c_str(), NULL,
                                                   get_children_args->args);
            }

            get_children_args->Release();
            get_children_args = NULL;
            return;
        }

        get_children_args->children_ctx.children_count = strings->count;
        get_children_args->children_ctx.children_n = 0;

        ZOOKEEPER_LOG_INFO("parent name: %s, children count: %d", get_children_args->parent_path.c_str(),
                           get_children_args->children_ctx.children_count);

        for (int i = 0; i < get_children_args->children_ctx.children_count; ++i)
        {
            ZOOKEEPER_LOG_INFO("%s", strings->data[i]);

            GetChildrenNodeArgs* get_children_node_args = GetChildrenNodeArgs::Create();
            if (NULL == get_children_node_args)
            {
                ZOOKEEPER_LOG_ERROR("failed to alloc memory");
                get_children_args->Release();
                get_children_args = NULL;
                return;
            }

            get_children_node_args->get_children_args = get_children_args;
            get_children_node_args->name = strings->data[i];

            char path[ZK_MAX_PATH_LEN + 1] = "";
            snprintf(path, sizeof(path), "%s/%s", get_children_args->parent_path.c_str(),
                     get_children_node_args->name.c_str());
            ZOOKEEPER_LOG_INFO("path: %s", path);

            get_children_node_args->path = path;

            int ret = zoo_aget(get_children_args->zh, path, get_children_args->watch_child, ChildrenDataCompletion,
                               get_children_node_args);
            if (ret != ZOK)
            {
                ZOOKEEPER_LOG_ERROR("zoo_aget failed, err msg: %s", zerror(ret));
                get_children_node_args->Release();
                get_children_node_args = NULL;

                get_children_args->Release();
                get_children_args = NULL;
                return;
            }
        }
    }
    else
    {
        get_children_args->Release();
        get_children_args = NULL;
        return;
    }
}

int GetChildren(const char* parent_path, int watch_parent, int watch_child, GetChildrenCompletionFunc completion_func,
                const void* args)
{
    if (NULL == ctx.zk_handle)
    {
        ZOOKEEPER_LOG_ERROR("zk_handle is null");
        return -1;
    }

    ZOOKEEPER_LOG_INFO("GetChildren called, parent_path: %s, pid: %d, thread id: %ld", parent_path, getpid(),
                       pthread_self());

    GetChildrenArgs* get_children_args = GetChildrenArgs::Create();
    if (NULL == get_children_args)
    {
        ZOOKEEPER_LOG_ERROR("failed to alloc memory");
        return -1;
    }

    get_children_args->zh = ctx.zk_handle;
    get_children_args->parent_path = parent_path;
    get_children_args->watch_child = watch_child;
    get_children_args->completion_func = completion_func;
    get_children_args->args = args;

    int ret = zoo_aget_children(ctx.zk_handle, parent_path, watch_parent, ChildrenStringsCompletion, get_children_args);
    if (ret != ZOK)
    {
        ZOOKEEPER_LOG_ERROR("zoo_aget_children failed, err msg: %s", zerror(ret));
        get_children_args->Release();
        get_children_args = NULL;
        return -1;
    }

    return 0;
}

int CreateEphemeralNode(const char* parent_path, const char* self_name)
{
    if (NULL == ctx.zk_handle)
    {
        ZOOKEEPER_LOG_ERROR("zk_handle is null");
        return -1;
    }

    char path[ZK_MAX_PATH_LEN + 1] = "";
    StrPrintf(path, sizeof(path), "%s/%s", parent_path, self_name);

    ZOOKEEPER_LOG_INFO("GetChildren called, path: %s, pid: %d, thread id: %ld", path, getpid(), pthread_self());

    int ret = zoo_exists(ctx.zk_handle, path, 0, NULL);
    if (ZOK == ret)
    {
        // 已经存在
        return 0;
    }

    if (ZNONODE == ret)
    {
        ret = zoo_create(ctx.zk_handle, path, NULL, 0, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, NULL, 0);
        if (ret != ZOK)
        {
            ZOOKEEPER_LOG_ERROR("zoo_create failed, err msg: %s", zerror(ret));
            return -1;
        }

        return 0;
    }

    ZOOKEEPER_LOG_ERROR("err msg: %s", zerror(ret));
    return -1;
}

struct GetNodeArgs
{
    std::string path;
    GetNodeCompletionFunc completion_func;
    const void* args;

    GetNodeArgs() : path("")
    {
        completion_func = NULL;
        args = NULL;
    }

    ~GetNodeArgs()
    {
    }

    static GetNodeArgs* Create()
    {
        return new GetNodeArgs();
    }

    void Release()
    {
        delete this;
    }
};

static void NodeDataCompletion(int rc, const char* value, int value_len, const struct Stat* stat, const void* data)
{
    ZOOKEEPER_LOG_INFO("NodeDataCompletion called, pid: %d, thread id: %ld, rc: %d", getpid(), pthread_self(), rc);
    GetNodeArgs* get_node_args = (GetNodeArgs*) data;

    if (ZOK == rc)
    {
        NodeCtx node_ctx;

        if (value != NULL && value_len > 0)
        {
            node_ctx.value.assign(value, value_len);
        }

        if (stat != NULL)
        {
            node_ctx.version = stat->version;
        }

        if (get_node_args->completion_func != NULL)
        {
            char node_name[ZK_MAX_PATH_LEN + 1] = "";
            GetFileName(node_name, sizeof(node_name), get_node_args->path.c_str());

            get_node_args->completion_func(get_node_args->path.c_str(), node_name, &node_ctx, get_node_args->args);
        }
    }

    get_node_args->Release();
    get_node_args = NULL;
}

int GetNode(const char* path, int watch, GetNodeCompletionFunc completion_func, const void* args)
{
    if (NULL == ctx.zk_handle)
    {
        ZOOKEEPER_LOG_ERROR("zk_handle is null");
        return -1;
    }

    ZOOKEEPER_LOG_INFO("GetChildren called, path: %s, pid: %d, thread id: %ld", path, getpid(), pthread_self());

    GetNodeArgs* get_node_args = GetNodeArgs::Create();
    if (NULL == get_node_args)
    {
        ZOOKEEPER_LOG_ERROR("failed to alloc memory");
        return -1;
    }

    get_node_args->path = path;
    get_node_args->completion_func = completion_func;
    get_node_args->args = args;

    int ret = zoo_aget(ctx.zk_handle, path, watch, NodeDataCompletion, get_node_args);
    if (ret != ZOK)
    {
        ZOOKEEPER_LOG_ERROR("zoo_aget failed, err msg: %s", zerror(ret));
        return -1;
    }

    return 0;
}
}
