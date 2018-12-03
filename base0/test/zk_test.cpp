#include "zk_test.h"
#include "simple_log.h"

//Zookeeper 所有的读操作——getData(), getChildren(), 和 exists() 都 可以设置监视(watch)，监视事件可以理解为一次性的触发器， 官方定义如下： a watch event is one-time trigger, sent to the client that set the watch, which occurs when the data for which the watch was set changes。对此需要作出如下理解：
//
//（一次性触发）One-time trigger
//当设置监视的数据发生改变时，该监视事件会被发送到客户端，例如，如果客户端调用了 getData("/znode1", true) 并且稍后 /znode1 节点上的数据发生了改变或者被删除了，客户端将会获取到 /znode1 发生变化的监视事件，而如果 /znode1 再一次发生了变化，除非客户端再次对 /znode1 设置监视，否则客户端不会收到事件通知。
//
//（发送至客户端）Sent to the client
//Zookeeper 客户端和服务端是通过 socket 进行通信的，由于网络存在故障，所以监视事件很有可能不会成功地到达客户端，监视事件是异步发送至监视者的，Zookeeper 本身提供了保序性(ordering guarantee)：即客户端只有首先看到了监视事件后，才会感知到它所设置监视的 znode 发生了变化(a client will never see a change for which it has set a watch until it first sees the watch event). 网络延迟或者其他因素可能导致不同的客户端在不同的时刻感知某一监视事件，但是不同的客户端所看到的一切具有一致的顺序。
//
//（被设置 watch 的数据）The data for which the watch was set
//    这意味着 znode 节点本身具有不同的改变方式。你也可以想象 Zookeeper 维护了两条监视链表：数据监视和子节点监视(data watches and child watches) getData() and exists() 设置数据监视，getChildren() 设置子节点监视。 或者，你也可以想象 Zookeeper 设置的不同监视返回不同的数据，getData() 和 exists() 返回 znode 节点的相关信息，而 getChildren() 返回子节点列表。因此，  setData() 会触发当前节点上所设置的数据监视(假定数据设置成功)，而一次成功的 create() 操作则会触发当前节点上所设置的数据监视以及父节点的子节点监视。一次成功的 delete() 操作将会触发当前节点的数据监视和子节点监视事件，同时也会触发该节点父节点的child watch。
//
//Zookeeper 中的监视是轻量级的，因此容易设置、维护和分发。当客户端与 Zookeeper 服务器端失去联系时，客户端并不会收到监视事件的通知，只有当客户端重新连接后，若在必要的情况下，以前注册的监视会重新被注册并触发，对于开发人员来说 这通常是透明的。只有一种情况会导致监视事件的丢失，即：通过 exists() 设置了某个 znode 节点的监视，但是如果某个客户端在此 znode 节点被创建和删除的时间间隔内与 zookeeper 服务器失去了联系，该客户端即使稍后重新连接 zookeeper服务器后也得不到事件通知。

//利用ZooKeeper的强一致性，能够保证在分布式高并发情况下节点创建的全局唯一性，即：同时有多个客户端请求创建 /currentMaster 节点，最终一定只有一个客户端请求能够创建成功。

//
//与监视类型(Watch Types)相关的常量，通常用作监视器回调函数的参数
//1 ZOO_CREATED_EVENT; // 节点被创建(此前该节点不存在)，通过 zoo_exists() 设置监视
//2 ZOO_DELETED_EVENT; // 节点被删除，通过 zoo_exists() 和 zoo_get() 设置监视
//3 ZOO_CHANGED_EVENT; // 节点发生变化，通过 zoo_exists() 和 zoo_get() 设置监视
//4 ZOO_CHILD_EVENT; // 子节点事件，通过zoo_get_children() 和 zoo_get_children2()设置监视
//-1 ZOO_SESSION_EVENT; // 会话事件
//-2 ZOO_NOTWATCHING_EVENT; // 监视被移除
//
//与连接状态 Stat 相关的常量，通常用作监视器回调函数的参数
//-112 ZOOAPI const int 	ZOO_EXPIRED_SESSION_STATE
//-113 ZOOAPI const int 	ZOO_AUTH_FAILED_STATE
//1 ZOOAPI const int 	ZOO_CONNECTING_STATE
//2 ZOOAPI const int 	ZOO_ASSOCIATING_STATE
//3 ZOOAPI const int 	ZOO_CONNECTED_STATE
//

//zookeeper的C客户端分为mt库和st库（多线程和单线程），一般操作都是以多线程库为主。
//
//多线程库分为三个线程，主线程，io线程和completion线程
//
//主线程就是调用API的线程，io线程负责网络通信，对异步请求和watch响应等，IO线程会发给completion线程，由completion线程异步完成

//enum ZOO_ERRORS {
//    ZOK = 0, /*!< Everything is OK */
//
//    /** System and server-side errors.
//     * This is never thrown by the server, it shouldn't be used other than
//     * to indicate a range. Specifically error codes greater than this
//     * value, but lesser than {@link #ZAPIERROR}, are system errors. */
//        ZSYSTEMERROR = -1,
//    ZRUNTIMEINCONSISTENCY = -2, /*!< A runtime inconsistency was found */
//    ZDATAINCONSISTENCY = -3, /*!< A data inconsistency was found */
//    ZCONNECTIONLOSS = -4, /*!< Connection to the server has been lost */
//    ZMARSHALLINGERROR = -5, /*!< Error while marshalling or unmarshalling data */
//    ZUNIMPLEMENTED = -6, /*!< Operation is unimplemented */
//    ZOPERATIONTIMEOUT = -7, /*!< Operation timeout */
//    ZBADARGUMENTS = -8, /*!< Invalid arguments */
//    ZINVALIDSTATE = -9, /*!< Invliad zhandle state */
//
//    /** API errors.
//     * This is never thrown by the server, it shouldn't be used other than
//     * to indicate a range. Specifically error codes greater than this
//     * value are API errors (while values less than this indicate a
//     * {@link #ZSYSTEMERROR}).
//     */
//        ZAPIERROR = -100,
//    ZNONODE = -101, /*!< Node does not exist */
//    ZNOAUTH = -102, /*!< Not authenticated */
//    ZBADVERSION = -103, /*!< Version conflict */
//    ZNOCHILDRENFOREPHEMERALS = -108, /*!< Ephemeral nodes may not have children */
//    ZNODEEXISTS = -110, /*!< The node already exists */
//    ZNOTEMPTY = -111, /*!< The node has children */
//    ZSESSIONEXPIRED = -112, /*!< The session has been expired by the server */
//    ZINVALIDCALLBACK = -113, /*!< Invalid callback specified */
//    ZINVALIDACL = -114, /*!< Invalid ACL specified */
//    ZAUTHFAILED = -115, /*!< Client authentication failed */
//    ZCLOSING = -116, /*!< ZooKeeper is closing */
//    ZNOTHING = -117, /*!< (not error) no server responses to process */
//    ZSESSIONMOVED = -118 /*!<session moved to another server, so operation is ignored */
//};

//
//zh	zookeeper 句柄(handle)
//type	事件类型(event type). *_EVENT 常量之一.
//state	连接状态(connection state). 状态值为 *_STATE 常量之一.
//path	触发监视事件的 znode 节点的路径，若为 NULL，则事件类型为 ZOO_SESSION_EVENT
//watcherCtx	监视器上下
//
// 全局监视器回调函数
void zktest_watcher_global(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx)
{
    LOG_CPP("in zktest_watcher_global, event happended, event type:" << type << ", state: " << state
                << ", znode path: " << (path != NULL ? path : "null") << ", watcher ctx: " << (char*) watcherCtx);

    if (ZOO_CREATED_EVENT == type)
    {
        LOG_CPP("create event");
    }
    else if (ZOO_DELETED_EVENT == type)
    {
        LOG_CPP("delete event");
    }
    else if (ZOO_CHANGED_EVENT == type)
    {
        LOG_CPP("change event");
    }
    else if (ZOO_SESSION_EVENT == type)
    {
        LOG_CPP("session event, state: " << state);

        if (state == ZOO_CONNECTED_STATE)
        {
            LOG_CPP("connected to zookeeper service successfully");
        }
        else if (state == ZOO_EXPIRED_SESSION_STATE)
        {
            LOG_CPP("zookeeper session expired");
        }
    }
}

//
//rc	异步函数调用返回的错误码，连接丢失/超时将触发该原型函数(此处指具有该函数原型的回调函数，下同)的调用，
// 并且错误码为 ZCONNECTIONLOSS --  Zookeeper 客户端与服务器端的连接丢失，
// 或者 ZOPERATIONTIMEOUT -- 连接超时；而与数据相关的事件也会触发该原型函数的调用，同时置相应的错误码，具体见后文(0 代异步函数调用成功)
//value	返回的字符串
//data	由调用者传入的指针，调用者可以通过该指针向回调函数传入自定义的参数，开发人员应负责此指针所指向内存的释放
//
void zktest_string_completion(int rc, const char* value, const void* data)
{
    LOG_CPP("in zktest_string_completion, rc: " << rc << ", value: " << value << ", data: " << (const char*) data);
}

void zktest_dump_stat(const struct Stat* stat)
{
    if (NULL == stat)
    {
        return;
    }

    time_t tctime = stat->ctime / 1000;
    time_t tmtime = stat->mtime / 1000;

    char tctimes[40] = "";
    char tmtimes[40] = "";

    ctime_r(&tmtime, tmtimes);
    ctime_r(&tctime, tctimes);

    LOG_C("ctime=%s czxid=%#lx mtime=%s mzxid=%#lx version=%#x aversion=%#x ephemeralOwner = %#lx",
          tctimes, stat->czxid, tmtimes, stat->mzxid, stat->version, stat->aversion, stat->ephemeralOwner);
}

//
//rc	异步函数调用返回的错误码，连接丢失/超时将触发该原型函数(此处指具有该函数原型的回调函数，下同)的调用，
// 并且错误码为 ZCONNECTIONLOSS --  Zookeeper 客户端与服务器端的连接丢失，
// 或者 ZOPERATIONTIMEOUT -- 连接超时；而与数据相关的事件也会触发该原型函数的调用，同时置相应的错误码，具体见后文(0 代异步函数调用成功)
//stat	指向与该 znode 节点相关的 Stat 信息，如果返回非 0 值(即异步调用函数出错)，stat 所指向的区域是未定义的，开发者不负责释放 stat 所指向的内存空间。
//data	由调用者传入的指针，调用者可以通过该指针向回调函数传入自定义的参数，开发人员应负责此指针所指向内存的释放。
//
void zktest_stat_completion(int rc, const struct Stat* stat, const void* data)
{
    LOG_CPP("in zktest_stat_completion, rc: " << rc << ", data: " << (const char*) data);

    if (ZOK == rc)
    {
        zktest_dump_stat(stat);
    }
}

//
//rc	异步函数调用返回的错误码，连接丢失/超时将触发该原型函数(此处指具有该函数原型的回调函数，下同)的调用，
// 并且错误码为 ZCONNECTIONLOSS --  Zookeeper 客户端与服务器端的连接丢失，
// 或者 ZOPERATIONTIMEOUT -- 连接超时；而与数据相关的事件也会触发该原型函数的调用，同时置相应的错误码，具体见后文(0 代异步函数调用成功)
//data	由调用者传入的指针，调用者可以通过该指针向回调函数传入自定义的参数，开发人员应负责此指针所指向内存的释放。
void zktest_void_completion(int rc, const void* data)
{
    LOG_CPP("in zktest_void_completion, rc: " << rc << ", data: " << (const char*) data);
}

ZkTest::ZkTest()
{
    zkhandle_ = NULL;
}

ZkTest::~ZkTest()
{

}

void ZkTest::SetUp()
{
//    const char* host = "127.0.0.1:2181,127.0.0.1:2182,"
//        "127.0.0.1:2183,127.0.0.1:2184,127.0.0.1:2185";
    const char* host = "127.0.0.1:2181";
    int timeout = 30000;

    zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);

    LOG_C("zookeeper_init begin");

    zkhandle_ = zookeeper_init("zk.moon.com:8081", zktest_watcher_global, timeout, 0, (void*) "hello zookeeper", 0);
    if (NULL == zkhandle_)
    {
        LOG_CPPE("Error when connecting to zookeeper servers...");
        FAIL();
    }

    LOG_C("zookeeper_init end");

    // Wait for asynchronous zookeeper call done.
    LOG_C("sleep");
    sleep(3);
}

void ZkTest::TearDown()
{
    if (zkhandle_ != NULL)
    {
        LOG_C("zookeeper_close begin");
        zookeeper_close(zkhandle_);
        LOG_C("zookeeper_close end");

        // Wait for asynchronous zookeeper call done.
        LOG_C("sleep");
        sleep(3);

        zkhandle_ = NULL;
    }
}

void ZkTest::Test001()
{

}

void ZkTest::Test002()
{
//    与 znode 访问权限有关的常量
//    const int ZOO_PERM_READ; //允许客户端读取 znode 节点的值以及子节点列表。
//    const int ZOO_PERM_WRITE; // 允许客户端设置 znode 节点的值。
//    const int ZOO_PERM_CREATE; //允许客户端在该 znode 节点下创建子节点。
//    const int ZOO_PERM_DELETE; //允许客户端删除子节点。
//    const int ZOO_PERM_ADMIN; //允许客户端执行 set_acl()。
//    const int ZOO_PERM_ALL; //允许客户端执行所有操作，等价与上述所有标志的或(OR) 。
//
//    与 ACL IDs 相关的常量
//    struct Id ZOO_ANYONE_ID_UNSAFE; //(‘world’,’anyone’)
//    struct Id ZOO_AUTH_IDS;// (‘auth’,’’)
//
//    三种标准的 ACL
//    struct ACL_vector ZOO_OPEN_ACL_UNSAFE; //(ZOO_PERM_ALL,ZOO_ANYONE_ID_UNSAFE)
//    struct ACL_vector ZOO_READ_ACL_UNSAFE; // (ZOO_PERM_READ, ZOO_ANYONE_ID_UNSAFE)
//    struct ACL_vector ZOO_CREATOR_ALL_ACL; //(ZOO_PERM_ALL,ZOO_AUTH_IDS)

//    zoo_create 函数标志
//    0
//    ZOO_EPHEMERAL 用来标识创建临时节点
//    ZOO_SEQUENCE 用来标识节点命名具有递增的后缀序号(一般是节点名称后填充 10 位字符的序号，如 /xyz0000000000, /xyz0000000001, /xyz0000000002, ...)
//
    LOG_C("zoo_acreate begin");

    int ret = zoo_acreate(zkhandle_, "/xyz", "hello", 5, &ZOO_OPEN_ACL_UNSAFE, 0, zktest_string_completion, "acreate");
    if (ret != ZOK)
    {
        LOG_CPPE("ret: " << ret);
        FAIL();
    }

    LOG_C("zoo_acreate end");

    // Wait for asynchronous zookeeper call done.
    LOG_C("sleep");
    sleep(3);

    LOG_C("zoo_aexists begin");

    ret = zoo_aexists(zkhandle_, "/xyz", 1, zktest_stat_completion, "aexists"); // 事件是一次性的，触发后需要再次监听
    if (ret != ZOK)
    {
        LOG_CPPE("ret: " << ret);
        FAIL();
    }

    LOG_C("zoo_aexists end");

    // Wait for asynchronous zookeeper call done.
    LOG_C("sleep");
    sleep(3);

    LOG_C("zoo_set begin");
    const char buf[] = "hello, world";
    int version = 0;

    ret = zoo_aset(zkhandle_, "/xyz", buf, strlen(buf), version, zktest_stat_completion, "aset");
    if (ret != ZOK)
    {
        LOG_CPPE("ret: " << ret);
        FAIL();
    }

    LOG_C("zoo_set end");

    // Wait for asynchronous zookeeper call done.
    LOG_C("sleep");
    sleep(3);

    LOG_C("zoo_aexists begin");

    ret = zoo_aexists(zkhandle_, "/xyz", 1, zktest_stat_completion, "aexists"); // 注意：事件是一次性的，触发后需要再次监听
    if (ret != ZOK)
    {
        LOG_CPPE("ret: " << ret);
        FAIL();
    }

    LOG_C("zoo_aexists end");

    // Wait for asynchronous zookeeper call done.
    LOG_C("sleep");
    sleep(3);

    LOG_C("zoo_adelete begin");

    ret = zoo_adelete(zkhandle_, "/xyz", -1, zktest_void_completion, "adelete");
    if (ret != ZOK)
    {
        LOG_CPPE("ret: " << ret);
        FAIL();
    }

    LOG_C("zoo_adelete end");

    // Wait for asynchronous zookeeper call done.
    LOG_C("sleep");
    sleep(3);
}

void ZkTest::Test003()
{
    LOG_C("zoo_create begin");

    char real_path[256] = "";

    int ret = zoo_create(zkhandle_, "/xyz", "hello", 5, &ZOO_OPEN_ACL_UNSAFE, 0, real_path, sizeof(real_path));
    if (ret != ZOK)
    {
        LOG_CPPE("ret: " << ret);
        FAIL();
    }

    LOG_CPP("zoo_acreate end, real path: " << real_path);

    LOG_C("zoo_exists begin");
    Stat stat;

    ret = zoo_exists(zkhandle_, "/xyz", 1, &stat); // 事件是一次性的，触发后需要再次监听
    if (ret != ZOK)
    {
        LOG_CPPE("ret: " << ret);
        FAIL();
    }

    LOG_C("zoo_exists end");

    LOG_C("zoo_set begin");
    const char buf[] = "hello, world";
    int version = 0;

    ret = zoo_set(zkhandle_, "/xyz", buf, strlen(buf), version);
    if (ret != ZOK)
    {
        LOG_CPPE("ret: " << ret);
        FAIL();
    }

    LOG_C("zoo_set end");

    LOG_C("zoo_exists begin");

    ret = zoo_exists(zkhandle_, "/xyz", 1, &stat); // 注意：事件是一次性的，触发后需要再次监听
    if (ret != ZOK)
    {
        LOG_CPPE("ret: " << ret);
        FAIL();
    }

    LOG_C("zoo_exists end");

    LOG_C("zoo_delete begin");

    ret = zoo_delete(zkhandle_, "/xyz", -1);
    if (ret != ZOK)
    {
        LOG_CPPE("ret: " << ret);
        FAIL();
    }

    LOG_C("zoo_delete end");
}

void ZkTest::Test004()
{

}

void ZkTest::Test005()
{

}

void ZkTest::Test006()
{
//    ZooKeeper使用ACL进行权限控制C++
//
//    最近使用Zookeeper作为配置管理服务，因为配置数据有很高的安全要求，需要有权限控制，也就是需要登录才能看到Zookeeper上面的数据。
//
//    Zookeeper对权限的控制是节点级别的，而且不继承，即对父节点设置权限，其子节点不继承父节点的权限。
//
//    Zookeeper提供了几种认证方式
//    * world：有个单一的ID，anyone，表示任何人。
//    * auth：不使用任何ID，表示任何通过验证的用户（是通过ZK验证的用户？连接到此ZK服务器的用户？）。
//    * digest：使用 用户名：密码 字符串生成MD5哈希值作为ACL标识符ID。权限的验证通过直接发送用户名密码字符串的方式完成，
//    * ip：使用客户端主机ip地址作为一个ACL标识符，ACL表达式是以 addr/bits 这种格式表示的。ZK服务器会将addr的前bits位与客户端地址的前bits位来进行匹配验证权限。
//
//    digest方式比较适合我们的业务，因此采用此种方式对Zookeeper进行权限控制。

    char scheme[10] = "digest";

    // user:base64encode(sha1(user:passwd))
    char usr_pa[100] = "test:V28q/NynI4JI3Rk54h0r8O5kMug=";

    struct ACL acl;
    acl.perms = ZOO_PERM_ALL;
    acl.id.scheme = scheme;
    acl.id.id = usr_pa;

    struct ACL_vector aclv;
    aclv.count = 1;
    aclv.data = &acl;

    char buffer[512] = "";
    zoo_create(zkhandle_, "/mynode", "cluster", strlen("cluster"), &aclv, 0, buffer,
               sizeof(buffer) - 1); // 创建节点的时候就指定了节点的访问权限

    // 在读(get)、删(delete)、创建子节点之前要先调用zoo_add_auth授权，exists不需要权限
    zoo_add_auth(zkhandle_, scheme, "test:test", 7, NULL, NULL);

    memset(buffer, 0, sizeof(buffer));
    int buflen = sizeof(buffer);
    struct Stat stat;
    int rc = zoo_get(zkhandle_, "/mynode", 0, buffer, &buflen, &stat);
    if (rc)
    {
        fprintf(stderr, "zoo_get Error %d for %d\n", rc, __LINE__);
    }
    printf("get buffer: %s\n", buffer);

    rc = zoo_delete(zkhandle_, "/mynode", -1); // 只有在没有子节点时才能被删除
    if (rc)
    {
        fprintf(stderr, "zoo_delete Error %d for %d\n", rc, __LINE__);
    }
    sleep(1000000);

//    其中对于希望使用的用户名、密码分别为user,passwd时： ACL的id中保存的字符串为user:base64encode(sha1(user:passwd)) 其中编码为：单字符8位，字符集为ASCII 在java中有DigestAuthenticationProvider.java模块插件可以进行此加密： 对应加密函数
//
//    static public String generateDigest(String idPassword)
//    throws NoSuchAlgorithmException {
//        String parts[] = idPassword.split(":", 2);
//        byte digest[] = MessageDigest.getInstance("SHA1").digest(
//        idPassword.getBytes());
//        return parts[0] + ":" + base64Encode(digest);
//    }
//
//    在zkCli.sh命令行中： 创建节点create -s /test null digest:test:V28q/NynI4JI3Rk54h0r8O5kMug=:rwcda 让zookeeper知道自己的权限addauth digest test:test 获取节点数据get /test
}

ADD_TEST_F(ZkTest, Test001);
ADD_TEST_F(ZkTest, Test002);
ADD_TEST_F(ZkTest, Test003);
ADD_TEST_F(ZkTest, Test004);
ADD_TEST_F(ZkTest, Test005);
ADD_TEST_F(ZkTest, Test006);
