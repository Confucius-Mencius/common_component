#ifndef BASE_INC_ZK_UTIL_H_
#define BASE_INC_ZK_UTIL_H_

#include <string>
#include <map>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include "zookeeper.h"

#ifdef __cplusplus
}
#endif // __cplusplus

// 注意：zookeeper的回调函数是在它自己的一个线程中执行的，这个线程访问的业务数据一般是需要加锁的。

namespace zk
{
static const int ZK_MAX_PATH_LEN = 255;

struct NodeCtx
{
    std::string value;
    int version;

    NodeCtx() : value("")
    {
        version = 0;
    }
};

typedef std::map<std::string, NodeCtx> NodeCtxMap; // node name ->

//一次成功的setData()会触发当前节点上所设置的数据监视
//一次成功的create()会触发当前节点上所设置的数据监视以及父节点的子节点监视
//一次成功的delete()会触发当前节点的数据监视和子节点监视事件，同时也会触发该节点父节点的child watch。

//注意：
//1，更新操作中的版本参数如果为-1，则表示更新操作针对任何版本均可。当更新版本不为-1，且不等于节点的目前版本，则更新失败。
//2，即使data[]的数据跟原有节点内的数据一样，进行更新操作之后，节点的信息也会发生变化，比如版本信息和更新时间。
//3，如果一个节点下面有子节点，需先删除子节点，然后才能删除父节点。

typedef enum
{
    EVENT_TYPE_MIN = 0,
    EVENT_TYPE_CONNECTED = 0,
    EVENT_TYPE_CHILDREN = 1, // 子结点有变化（增加、减少）
    EVENT_TYPE_CHANGED = 2, // 结点有变化（修改、删除）
    EVENT_TYPE_MAX = 3
} EventType;

typedef void (* EventFunc)(const char* path, EventType event_type, const void* args);
typedef void (* GetChildrenCompletionFunc)(const char* path, const NodeCtxMap* node_ctx_map, const void* args);
typedef void (* GetNodeCompletionFunc)(const char* path, const char* node_name, const NodeCtx* node_ctx,
                                       const void* args);

struct Event
{
    const void* args;
    EventFunc func;

    Event()
    {
        args = NULL;
        func = NULL;
    }

    ~Event()
    {
    }
};

struct EventConf
{
    Event connected_event;
    Event children_event;
    Event changed_event;

    EventConf() : connected_event(), children_event(), changed_event()
    {
    }

    ~EventConf()
    {
    }
};

struct Conf
{
    std::string zk_host; // 集群地址
    int zk_timeout; // 连接超时，单位：秒
    int zk_log_level; // ZOO_LOG_LEVEL_ERROR=1,ZOO_LOG_LEVEL_WARN=2,ZOO_LOG_LEVEL_INFO=3,ZOO_LOG_LEVEL_DEBUG=4
    std::string zk_log_file_path; // /tmp/zk.log
    std::string zk_user_passwd; // user:password
    EventConf event_conf;

    Conf() : zk_host(), zk_log_file_path(""), zk_user_passwd(""), event_conf()
    {
        zk_timeout = 10;
        zk_log_level = ZOO_LOG_LEVEL_DEBUG;
    }

    ~Conf()
    {
    }
};

int Initialize(const Conf* conf);
void Finalize();
int GetChildren(const char* parent_path, int watch_parent, int watch_child,
                GetChildrenCompletionFunc completion_func, const void* args);
int CreateEphemeralNode(const char* parent_path, const char* self_name);
int GetNode(const char* path, int watch, GetNodeCompletionFunc completion_func, const void* args);
}

#endif // BASE_INC_ZK_UTIL_H_
