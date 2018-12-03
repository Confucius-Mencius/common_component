#include "mongo_c_test.h"
#include "bson/bson.h"

// TODO 不定义这个宏编不过，怎么回事？
#define BSON_GNUC_PURE

#include "mongoc.h"

MongoCTest::MongoCTest()
{
}

MongoCTest::~MongoCTest()
{
}

//2. api操作
//    Libmongoc提供一系列的存储访问接口，以及存储和传输结构使用类型。其提供的为同步接口。
//2.1. 创建连接
//    使用libmongoc驱动创建到mongo实例的连接
void MongoCTest::Test001()
{
    mongoc_client_t* client;
    mongoc_database_t* database;
    mongoc_collection_t* collection;
    bson_t* command;
    bson_t reply;
    bson_t* insert;
    bson_error_t error;
    char* str;
    bool retval;
    /*
     * Required to initialize libmongoc's internals
     */
    mongoc_init();//初始化libmongoc驱动
    /*
     * Create a new client instance
     */
    client = mongoc_client_new("mongodb://localhost:27017");//创建连接对象
    /*
     * Get a handle on the database "db_name" and collection "coll_name"
     */
    database = mongoc_client_get_database(client, "db_name");//获取数据库
    collection = mongoc_client_get_collection(client, "db_name", "coll_name");//获取指定数据库和集合
    /*
     * Do work. This example pings the database, prints the result as JSON and
     * performs an insert
     */
    command = BCON_NEW ("ping", BCON_INT32(1));
    retval = mongoc_client_command_simple(client, "admin", command, NULL, &reply, &error);//执行命令
    if (!retval)
    {
        fprintf(stderr, "%s\n", error.message);
        FAIL();
    }
    str = bson_as_json(&reply, NULL);
    printf("%s\n", str);
    insert = BCON_NEW ("hello", BCON_UTF8("world"));//字段为hello，值为world字符串
    if (!mongoc_collection_insert(collection, MONGOC_INSERT_NONE, insert, NULL, &error))
    {
        //插入文档
        fprintf(stderr, "%s\n", error.message);
    }
    bson_destroy(insert);
    bson_destroy(&reply);
    bson_destroy(command);
    bson_free(str);
    /*
     * Release our handles and clean up libmongoc
     */
    mongoc_collection_destroy(collection);//释放表对象
    mongoc_database_destroy(database);//释放数据库对象
    mongoc_client_destroy(client);//释放连接对象
    mongoc_cleanup();//释放libmongoc驱动
}

//2.2. 文档
//    MongoDB使用了BSON这种结构来存储数据和网络数据交换。把这种格式转化成一文档这个概念，这里的一个文档也可以理解成关系数据库中的一条记录，只是这里的文档的变化更丰富一些，如文档可以嵌套。MongoDB以BSON做为其存储结构的一种重要原因是其可遍历性。
//2.3. 创建文档
//    mongodb  c驱动使用libbson来创建BSON文档。有几种构建文档的方式：使用BSON插入键值对，或者解析json。
//2.3.1. 使用BSON插入键值对
void MongoCTest::Test002()
{
    bson_t* document;
    bson_t child;
    char* str;
    document = bson_new();
    /*
     * Append {"hello" : "world"} to the document.
     * Passing -1 for the length argument tells libbson to calculate the string length.
     */
    bson_append_utf8(document, "hello", -1, "world", -1);
    /*
     * For convenience, this macro is equivalent.
     */
    BSON_APPEND_UTF8 (document, "hello", "world");
    /*
     * Begin a subdocument.
     */
    BSON_APPEND_DOCUMENT_BEGIN (document, "subdoc", &child);
    BSON_APPEND_UTF8 (&child, "subkey", "value");
    bson_append_document_end(document, &child);
    /*
     * Print the document as a JSON string.
     */
    str = bson_as_json(document, NULL);
    printf("%s\n", str);
    bson_free(str);
    /*
     * Clean up allocated bson documents.
     */
    bson_destroy(document);
}

//可以查看libbson文档来查看能被添加到bson对象的数据类型。
//2.3.2. 使用BCON构造函数来构建BSON对象
void MongoCTest::Test003()
{
    bson_t* doc;
    char* str;
    doc = BCON_NEW ("name", BCON_UTF8("Babe Ruth"),
                    "statistics", "{",
                    "batting_average", BCON_DOUBLE(.342),
                    "hits", BCON_INT32(2873),
                    "home_runs", BCON_INT32(714),
                    "rbi", BCON_INT32(2213),
                    "}",
                    "nicknames", "[",
                    BCON_UTF8("the Sultan of Swat"),
                    BCON_UTF8("the Bambino"),
                    "]");
    str = bson_as_json(doc, NULL);
    printf("%s\n", str);
    bson_free(str);
    bson_destroy(doc);
}

//2.3.3. 从json字符串创建bson对象
void MongoCTest::Test004()
{
    bson_error_t error;
    bson_t* bson;
    char* string;

    const char* json = "{\"hello\": \"world\"}";
    bson = bson_new_from_json((const uint8_t*) json, -1, &error);
    if (!bson)
    {
        fprintf(stderr, "%s\n", error.message);
        FAIL();
    }
    string = bson_as_json(bson, NULL);
    printf("%s\n", string);
    bson_free(string);
}

//2.4. 基本CRUD操作
//2.4.1. 插入文档
//    插入文档{ "_id" : ObjectId("55ef43766cb5f36a3bae6ee4"), "hello" : "world" }
void MongoCTest::Test005()
{
    mongoc_client_t* client;
    mongoc_collection_t* collection;
    bson_error_t error;
    bson_oid_t oid;
    bson_t* doc;
    mongoc_init();
    client = mongoc_client_new("mongodb://localhost:27017/");
    if (client != NULL)
    {
        collection = mongoc_client_get_collection(client, "mydb", "mycoll");//插入到数据库mydb的集合mycoll中
        doc = bson_new();
        bson_oid_init(&oid, NULL);
        BSON_APPEND_OID (doc, "_id", &oid);//文档id
        BSON_APPEND_UTF8 (doc, "hello", "world");//hello字段
        if (!mongoc_collection_insert(collection, MONGOC_INSERT_NONE, doc, NULL, &error))
        {
            fprintf(stderr, "%s\n", error.message);
        }
        bson_destroy(doc);
        mongoc_collection_destroy(collection);
        mongoc_client_destroy(client);
    }

    mongoc_cleanup();
}

//2.4.2. 查找文档
//使用函数mongc_collection_find()来返回符合的结果文档的游标，需要传入bson查询对象。通过游标遍历结果集。
//bson查询对象，比如：{ "color" : "red" }，则会获取到字段为"color" 和值为"red" 的所有的文档。空查询对象则会获取所有的结果文档。
void MongoCTest::Test006()
{
    mongoc_client_t* client;
    mongoc_collection_t* collection;
    mongoc_cursor_t* cursor;
    const bson_t* doc;
    bson_t* query;
    char* str;
    mongoc_init();
    client = mongoc_client_new("mongodb://localhost:27017/");

    if (client != NULL)
    {

    }
    collection = mongoc_client_get_collection(client, "mydb", "mycoll");
    query = bson_new();
    cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);//空查询对象获取所有的文档

    while (mongoc_cursor_next(cursor, &doc))
    {
        str = bson_as_json(doc, NULL);
        printf("%s\n", str);
        bson_free(str);
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);
    mongoc_cleanup();
}

//2.4.3. 更新文档
//    先插入一个文档，根据文档id来更新文档。
void MongoCTest::Test007()
{
    mongoc_collection_t* collection;
    mongoc_client_t* client;
    bson_error_t error;
    bson_oid_t oid;
    bson_t* doc = NULL;
    bson_t* update = NULL;
    bson_t* query = NULL;
    mongoc_init();
    client = mongoc_client_new("mongodb://localhost:27017/");//创建连接
    collection = mongoc_client_get_collection(client, "mydb", "mycoll");//指定数据库和集合（即表）
    bson_oid_init(&oid, NULL);
    doc = BCON_NEW ("_id", BCON_OID(&oid),
                    "key",
                    BCON_UTF8("old_value"));//先插入{ "_id" : ObjectId("55ef549236fe322f9490e17b"), "key" : "old_value" }
    if (!mongoc_collection_insert(collection, MONGOC_INSERT_NONE, doc, NULL, &error))
    {
        fprintf(stderr, "%s\n", error.message);
        goto fail;
    }
    query = BCON_NEW ("_id", BCON_OID(&oid));//条件为id "_id" : ObjectId("55ef549236fe322f9490e17b")
    update = BCON_NEW ("$set", "{",
                       "key", BCON_UTF8("new_value"),
                       "updated", BCON_BOOL(true),
                       "}");
    if (!mongoc_collection_update(collection, MONGOC_UPDATE_NONE, query, update, NULL, &error))
    {
        fprintf(stderr, "%s\n", error.message);
        goto fail;
    }
fail:
    if (doc)
    {
        bson_destroy(doc);
    }
    if (query)
    {
        bson_destroy(query);
    }
    if (update)
    {
        bson_destroy(update);
    }
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);
    mongoc_cleanup();
}

//2.4.4. 删除文档
//    删除符合条件 {"hello" : "world"}的文档
void MongoCTest::Test008()
{
    mongoc_client_t* client;
    mongoc_collection_t* collection;
    bson_error_t error;
    bson_oid_t oid;
    bson_t* doc;
    mongoc_init();
    client = mongoc_client_new("mongodb://localhost:27017/");//创建连接到指定ip和端口的mongodb实例
    collection = mongoc_client_get_collection(client, "mydb", "mycoll");//指定数据库和集合
    doc = bson_new();
    bson_oid_init(&oid, NULL);
    BSON_APPEND_OID (doc, "_id", &oid);
    BSON_APPEND_UTF8 (doc, "hello", "world");//构建文档对象
    if (!mongoc_collection_insert(collection, MONGOC_INSERT_NONE, doc, NULL, &error))
    {
        fprintf(stderr, "Insert failed: %s\n", error.message);//插入文档
    }
    bson_destroy(doc);
    doc = bson_new();
    BSON_APPEND_OID (doc, "_id", &oid);
    if (!mongoc_collection_remove(collection, MONGOC_REMOVE_SINGLE_REMOVE, doc, NULL, &error))
    {
        fprintf(stderr, "Delete failed: %s\n", error.message);
    }
    bson_destroy(doc);
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);
    mongoc_cleanup();
}

//2.4.5. 计算文档个数
//    计算符合条件 {"hello" : "world"} 的文档的个数，只要包含该条件的文档都会计数。
void MongoCTest::Test009()
{
    mongoc_client_t* client;
    mongoc_collection_t* collection;
    bson_error_t error;
    bson_t* doc;
    int64_t count;
    mongoc_init();
    client = mongoc_client_new("mongodb://localhost:27017/");//创建连接
    collection = mongoc_client_get_collection(client, "mydb", "mycoll");//指定数据库和集合
    doc = bson_new_from_json((const uint8_t*) "{\"hello\" : \"world\"}", -1, &error);  //构造计数条件
    count = mongoc_collection_count(collection, MONGOC_QUERY_NONE, doc, 0, 0, NULL, &error);
    if (count < 0)
    {
        fprintf(stderr, "%s\n", error.message);
    }
    else
    {
        printf("%ld\n", count);
    }
    bson_destroy(doc);
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);
    mongoc_cleanup();
}

//2.5. 执行命令
//    函数mongoc_collection_command_simple可执行MongoDB 命令，传入执行命令的结果，函数返回执行的结果成功与否。
void MongoCTest::Test010()
{
    mongoc_client_t* client;
    mongoc_collection_t* collection;
    bson_error_t error;
    bson_t* command;
    bson_t reply;
    char* str;
    mongoc_init();
    client = mongoc_client_new("mongodb://localhost:27017/");
    collection = mongoc_client_get_collection(client, "mydb", "mycoll");
    command = BCON_NEW ("collStats", BCON_UTF8("mycoll"));
    if (mongoc_collection_command_simple(collection, command, NULL, &reply, &error))
    {
        str = bson_as_json(&reply, NULL);//输出执行命令后的结果
        printf("%s\n", str);
        bson_free(str);
    }
    else
    {
        fprintf(stderr, "Failed to run command: %s\n", error.message);
    }
    bson_destroy(command);
    bson_destroy(&reply);
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);
    mongoc_cleanup();
}

//2.6. 线程安全
//    MongoDB C Driver 多数操作都不是线程安全的，需要自己来保证。
//提供了线程安全的连接来获取连接对象。

static const int N_THREADS = 10;

static void*
worker(void* data)
{
    mongoc_client_pool_t* pool = (mongoc_client_pool_t*) data;
    mongoc_client_t* client;
    client = mongoc_client_pool_pop(pool);//从连接池中获取连接对象
    /* Do something... */
    mongoc_client_pool_push(pool, client);
    return NULL;
}

void MongoCTest::Test011()
{
    mongoc_client_pool_t* pool;
    mongoc_uri_t* uri;
    pthread_t threads[N_THREADS];
    mongoc_init();
    uri = mongoc_uri_new("mongodb://localhost/");//url指定ip地址，没有指定
    pool = mongoc_client_pool_new(uri);
    int i;

    for (i = 0; i < N_THREADS; i++)
    {
        pthread_create(&threads[i], NULL, worker, pool);
    }
    for (i = 0; i < N_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }
    mongoc_client_pool_destroy(pool);
    mongoc_uri_destroy(uri);
    mongoc_cleanup();
}

static int Test012_(int argc, const char* argv[])
{
    mongoc_gridfs_t* gridfs;
    mongoc_gridfs_file_t* file;
    mongoc_gridfs_file_list_t* list;
    mongoc_gridfs_file_opt_t opt = {0};
    mongoc_client_t* client;
    mongoc_stream_t* stream;
    bson_t query;
    bson_t child;
    bson_error_t error;
    ssize_t r;
    char buf[4096];
    mongoc_iovec_t iov;
    const char* filename;
    const char* command;
    if (argc < 2)
    {
        fprintf(stderr, "usage - %s command ...\n", argv[0]);
        return 1;
    }
    mongoc_init();
    iov.iov_base = (void*) buf;
    iov.iov_len = sizeof buf;
    /* connect to localhost client */
    client = mongoc_client_new("mongodb://127.0.0.1:27017");//创建连接
    assert(client);//检查创建连接结果
    /* grab a gridfs handle in test prefixed by fs */
    gridfs = mongoc_client_get_gridfs(client, "test", "fs", &error);
    assert(gridfs);
    command = argv[1];
    filename = argv[2];
    if (strcmp(command, "read") == 0)
    {
        //读取指定文档
        if (argc != 3)
        {
            fprintf(stderr, "usage - %s read filename\n", argv[0]);
            return 1;
        }
        file = mongoc_gridfs_find_one_by_filename(gridfs, filename, &error);
        assert(file);
        stream = mongoc_stream_gridfs_new(file);
        assert(stream);
        for (;;)
        {
            r = mongoc_stream_readv(stream, &iov, 1, -1, 0);
            assert (r >= 0);
            if (r == 0)
            {
                break;
            }
            if (fwrite(iov.iov_base, 1, r, stdout) != (size_t) r)
            {
                MONGOC_ERROR ("Failed to write to stdout. Exiting.\n");
                exit(1);
            }
        }
        mongoc_stream_destroy(stream);
        mongoc_gridfs_file_destroy(file);
    }
    else if (strcmp(command, "list") == 0)
    {
        //列举所有文档
        bson_init(&query);
        bson_append_document_begin(&query, "$orderby", -1, &child);
        bson_append_int32(&child, "filename", -1, 1);
        bson_append_document_end(&query, &child);
        bson_append_document_begin(&query, "$query", -1, &child);
        bson_append_document_end(&query, &child);
        list = mongoc_gridfs_find(gridfs, &query);
        bson_destroy(&query);
        while ((file = mongoc_gridfs_file_list_next(list)))
        {
            const char* name = mongoc_gridfs_file_get_filename(file);
            printf("%s\n", name ? name : "?");
            mongoc_gridfs_file_destroy(file);
        }
        mongoc_gridfs_file_list_destroy(list);
    }
    else if (strcmp(command, "write") == 0)
    {
        //写文档
        if (argc != 4)
        {
            fprintf(stderr, "usage - %s write filename input_file\n", argv[0]);
            return 1;
        }
        stream = mongoc_stream_file_new_for_path(argv[3], O_RDONLY, 0);
        assert (stream);
        opt.filename = filename;
        file = mongoc_gridfs_create_file_from_stream(gridfs, stream, &opt);
        assert(file);
        mongoc_gridfs_file_save(file);
        mongoc_gridfs_file_destroy(file);
    }
    else
    {
        fprintf(stderr, "Unknown command");
        return 1;
    }
    mongoc_gridfs_destroy(gridfs);
    mongoc_client_destroy(client);
    mongoc_cleanup();

    return 0;
}

//2.7. 大结构文件系统
//    GridFS是一种在MongoDB中存储大二进制文件的机制。使用GridFS存文件有如下几个原因：
//利用Grid可以简化需求。要是已经用了MongoDB，GridFS就可以不需要使用独立文件存储架构。
//GridFS会直接利用业已建立的复制或分片机制，所以对于文件存储来说故障恢复和扩展都很容易。
//GridFS可以避免用于存储用户上传内容的文件系统出现的某些问题。例如，GridFS在同一个目录下放置大量的文件是没有任何问题的。
//GridFS不产生磁盘碎片，因为MongoDB分配数据文件空间时以2GB为一块。
//使用场景:
//1) 有大量的上传图片(用户上传或者系统本身的文件发布等)
//2) 文件的量级处于飞速增长,有可能打到单机操作系统自己的文件系统的查询性能瓶颈,甚至超过单机硬盘的扩容范围.
//3) 文件的备份(不适用gridfs这种三方也可以做,但是不尽方便),文件系统访问的故障转移和修复..
//4) 文件的索引,存储除文件本身以外还需要关联更多的元数据信息(比如,不仅仅存储文件,还要保存一些文件的发布式作者/发布时间/文件tag属性等等自定义信息)并且需要索引的。
//5) 基于4),对文件的分类模糊,如果采用操作系统的文件系统,文件夹分类关系混乱或者无法分类时
//6) 当前系统是基于web的,对图片的访问根据url了规则路由的..(如搭配nginx用，让nginx直接读取gridfs的文件)
//7) 文件尺寸较小,而且众多,且文件有可能被迁移/删除等
//8)用于存储和恢复那些超过16M（BSON文件限制）的文件
//    接口代码如下：
//mongoc_gridfs_t提供的是MongoDB 的gridfs 文件系统的接口。地理信息文件系统包含gridfs_files 和gridfs_file_lists以及相关的api。
//mongoc_gridfs_t 是非线程安全的。释放mongoc_gridfs_t之前需要先释放mongoc_gridfs_file_t和mongoc_gridfs_file_list_t
void MongoCTest::Test012()
{

}

ADD_TEST_F(MongoCTest, Test001);
ADD_TEST_F(MongoCTest, Test002);
ADD_TEST_F(MongoCTest, Test003);
ADD_TEST_F(MongoCTest, Test004);
ADD_TEST_F(MongoCTest, Test005);
ADD_TEST_F(MongoCTest, Test006);
ADD_TEST_F(MongoCTest, Test007);
ADD_TEST_F(MongoCTest, Test008);
ADD_TEST_F(MongoCTest, Test009);
ADD_TEST_F(MongoCTest, Test010);
ADD_TEST_F(MongoCTest, Test011);
ADD_TEST_F(MongoCTest, Test012);
