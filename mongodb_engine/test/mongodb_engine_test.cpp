#include "mongodb_engine_test.h"
#include "log_util.h"
#include "console_log_engine.h"
#include "mem_util.h"
#include "bsoncxx/builder/basic/document.hpp"
#include "bsoncxx/json.hpp"

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::basic::kvp;

ConsoleLogEngine log_engine;
LogEngineInterface* g_log_engine = &log_engine;

static const char* db_name = "game_db";
static const char* coll_name = "player";

std::pair<mongocxx::instance*, mongocxx::pool*> MongodbEngineTest::mongodb_ctx_;

MongodbEngineTest::MongodbEngineTest()
{
    mongodb_engine_ = NULL;
}

MongodbEngineTest::~MongodbEngineTest()
{
}

void MongodbEngineTest::SetUpTestCase()
{
    mongodb_ctx_ = MongodbEngineInterface::GlobalInitialize("mongodb://127.0.0.1:27017");
}

void MongodbEngineTest::TearDownTestCase()
{
    MongodbEngineInterface::GlobalFinalize(mongodb_ctx_);
}

void MongodbEngineTest::SetUp()
{
    if (mongodb_engine_loader_.Load("../libmongodb_engine.so") != 0)
    {
        FAIL() << mongodb_engine_loader_.GetLastErrMsg();
    }

    mongodb_engine_ = (MongodbEngineInterface*) mongodb_engine_loader_.GetModuleInterface(0);
    if (NULL == mongodb_engine_)
    {
        FAIL() << mongodb_engine_loader_.GetLastErrMsg();
    }

    MongodbEngineCtx mongodb_engine_ctx;
    mongodb_engine_ctx.mongodb_pool = mongodb_ctx_.second;

    if (mongodb_engine_->Initialize(&mongodb_engine_ctx) != 0)
    {
        FAIL() << mongodb_engine_->GetLastErrMsg();
    }

    if (mongodb_engine_->Activate() != 0)
    {
        FAIL() << mongodb_engine_->GetLastErrMsg();
    }
}

void MongodbEngineTest::TearDown()
{
    SAFE_DESTROY_MODULE(mongodb_engine_, mongodb_engine_loader_);
}

void MongodbEngineTest::Test001()
{
    std::cout << getpid() << " " << pthread_self() << std::endl;

    // create index
    std::map<std::string, Variant> kvs;
    kvs["player_id"] = Variant(1);

    bsoncxx::document::value index = mongodb_engine_->Kvs2Doc(kvs);

    mongocxx::options::index index_options{};
    index_options.unique(true);

    if (mongodb_engine_->CreateIndex(db_name, coll_name, index, index_options) != 0)
    {
        FAIL();
    }
}

void MongodbEngineTest::Test002()
{
    std::cout << getpid() << " " << pthread_self() << std::endl;

    // insert
    std::map<std::string, Variant> kvs;

    kvs["player_id"] = Variant(1);
    kvs["i32"] = Variant(100);
    kvs["i64"] = Variant(INT64_C(9999));
    kvs["f32"] = Variant(0.5f);
    kvs["f64"] = Variant(9.9);
    kvs["str"] = Variant(Variant::TYPE_STR, "str1", 4);
    kvs["bin"] = Variant(Variant::TYPE_BIN, "bin1", 4);

    bsoncxx::document::value doc = mongodb_engine_->Kvs2Doc(kvs);

    if (mongodb_engine_->Insert(db_name, coll_name, doc, mongocxx::options::insert{}) != 0)
    {
        FAIL();
    }

    // 在unique index字段第二次插入就失败
    if (mongodb_engine_->Insert(db_name, coll_name, doc, mongocxx::options::insert{}) != 0)
    {
        FAIL();
    }

    // insert many
}

void MongodbEngineTest::Test003()
{
    // find
    {
        std::map<std::string, Variant> kvs_filter;
        kvs_filter["player_id"] = Variant(1);

        bsoncxx::document::value filter = mongodb_engine_->Kvs2Doc(kvs_filter);

        // 结果中去掉_id字段
        mongocxx::options::find options{};
        options.projection(document{} << "_id" << 0 << finalize);

        std::vector<bsoncxx::document::view> results;

        if (mongodb_engine_->Find(results, db_name, coll_name, filter, options) != 0)
        {
            FAIL();
        }

        for (std::vector<bsoncxx::document::view>::const_iterator it = results.begin(); it != results.end(); ++it)
        {
            std::map<std::string, Variant> kvs = mongodb_engine_->Doc2Kvs(*it);
            for (std::map<std::string, Variant>::const_iterator it_kvs = kvs.begin(); it_kvs != kvs.end(); ++it_kvs)
            {
                LOG_TRACE("key: " << it_kvs->first << ", value: " << it_kvs->second);
            }

            std::string json(mongodb_engine_->Doc2Json(*it));
            LOG_TRACE("json: " << json);
        }
    }

    {
        std::map<std::string, Variant> kvs_filter;
        kvs_filter["player_id"] = Variant(1);

        bsoncxx::document::value filter = mongodb_engine_->Kvs2Doc(kvs_filter);

        std::vector<bsoncxx::document::view> results;

        if (mongodb_engine_->Find(results, db_name, coll_name, filter, mongocxx::options::find{}) != 0)
        {
            FAIL();
        }

        for (std::vector<bsoncxx::document::view>::const_iterator it = results.begin(); it != results.end(); ++it)
        {
            std::map<std::string, Variant> kvs = mongodb_engine_->Doc2Kvs(*it);
            for (std::map<std::string, Variant>::const_iterator it_kvs = kvs.begin(); it_kvs != kvs.end(); ++it_kvs)
            {
                LOG_TRACE("key: " << it_kvs->first << ", value: " << it_kvs->second);
            }

            std::string json(mongodb_engine_->Doc2Json(*it));
            LOG_TRACE("json: " << json);

            // 取一个doc中的各个字段
            bsoncxx::document::view::const_iterator it_player_id = (*it).find("player_id");
            if (it_player_id != (*it).end() && it_player_id->length() > 0)
            {
                LOG_TRACE(it_player_id->get_int32().value);
            }

            bsoncxx::document::view::const_iterator it_str_data = (*it).find("str");
            if (it_str_data != (*it).end() && it_str_data->length() > 0)
            {
                LOG_TRACE(it_str_data->get_utf8().value.to_string());
            }

            bsoncxx::document::view::const_iterator it_xxxx = (*it).find("xxxx");
            if (it_str_data != (*it).end())
            {
                ASSERT_EQ(0, (int) it_xxxx->length()); // 注意：不存在的字段，length为０
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
    // 找不到的情况
    {
        std::map<std::string, Variant> kvs_filter;
        kvs_filter["player_id"] = Variant(2);

        bsoncxx::document::value filter = mongodb_engine_->Kvs2Doc(kvs_filter);

        std::vector<bsoncxx::document::view> results;

        if (mongodb_engine_->Find(results, db_name, coll_name, filter, mongocxx::options::find{}) != 0)
        {
            FAIL();
        }

        for (std::vector<bsoncxx::document::view>::const_iterator it = results.begin(); it != results.end(); ++it)
        {
            // 取一个doc中的各个字段
            bsoncxx::document::view::const_iterator it_player_id = (*it).find("player_id");
            if (it_player_id != (*it).end())
            {
                LOG_TRACE(it_player_id->get_int32().value);
            }

            bsoncxx::document::view::const_iterator it_str_data = (*it).find("str");
            if (it_str_data != (*it).end())
            {
                LOG_TRACE(it_str_data->get_utf8().value.to_string());
            }
        }
    }
}

void MongodbEngineTest::Test004()
{
    // update
    {
        // 如果set的值没有变化，update操作仍然返回成功
        std::map<std::string, Variant> kvs_filter;
        kvs_filter["player_id"] = Variant(1);

        bsoncxx::document::value filter = mongodb_engine_->Kvs2Doc(kvs_filter);
        bsoncxx::document::value update =
            document{} << "$set" << open_document << "str" << "xxxxxxxxxx" << close_document << finalize;

        if (mongodb_engine_->Update(db_name, coll_name, filter, update, mongocxx::options::update{}) != 0)
        {
            FAIL();
        }
    }

    // 要更新的字段不存在时，update操作会自动增加该字段和值
    {
        std::map<std::string, Variant> kvs_filter;
        kvs_filter["player_id"] = Variant(1);

        bsoncxx::document::value filter = mongodb_engine_->Kvs2Doc(kvs_filter);
        bsoncxx::document::value update =
            document{} << "$set" << open_document << "strxx" << "xxxxxxxxxx" << close_document << finalize;

        if (mongodb_engine_->Update(db_name, coll_name, filter, update, mongocxx::options::update{}) != 0)
        {
            FAIL();
        }
    }

    // filter描述的doc不存在时，update操作不会增加一个新doc
    {
        std::map<std::string, Variant> kvs_filter;
        kvs_filter["player_id"] = Variant(2);

        bsoncxx::document::value filter = mongodb_engine_->Kvs2Doc(kvs_filter);
        bsoncxx::document::value update =
            document{} << "$set" << open_document << "str" << "xxxxxxxxxx" << close_document << finalize;

        if (mongodb_engine_->Update(db_name, coll_name, filter, update, mongocxx::options::update{}) != 0)
        {
            FAIL();
        }
    }

    // 一次update多个字段,stream方式
    {
        std::map<std::string, Variant> kvs_filter;
        kvs_filter["player_id"] = Variant(1);

        bsoncxx::document::value filter = mongodb_engine_->Kvs2Doc(kvs_filter);
        bsoncxx::document::value update =
            document{} << "$set" << open_document << "str" << "yyy" << "strxx" << "yyyy" << close_document << finalize;

        if (mongodb_engine_->Update(db_name, coll_name, filter, update, mongocxx::options::update{}) != 0)
        {
            FAIL();
        }
    }

    // 一次update多个字段,basic方式
    {
        std::map<std::string, Variant> kvs_filter;
        kvs_filter["player_id"] = Variant(1);

        bsoncxx::document::value filter = mongodb_engine_->Kvs2Doc(kvs_filter);

        bsoncxx::builder::basic::document basic_builder{};
        basic_builder.append(kvp("str", "ddddd"));

        bsoncxx::types::b_binary bin_data;
        bin_data.bytes = (const uint8_t*) "ddddd";
        bin_data.size = 5;

        basic_builder.append(kvp("bin", bin_data));

        bsoncxx::document::value doc = basic_builder.extract();

        bsoncxx::builder::basic::document basic_builder2{};
        basic_builder2.append(kvp("$set", doc));
        LOG_TRACE(bsoncxx::to_json(basic_builder2.extract()));

//        if (mongodb_engine_->Update(db_name, coll_name, filter, basic_builder2.extract()) != 0)
//        {
//            FAIL(); // 这种方式不行
//        }

        bsoncxx::document::value update = document{} << "$set" << doc << finalize;

        if (mongodb_engine_->Update(db_name, coll_name, filter, update, mongocxx::options::update{}) != 0)
        {
            FAIL();
        }

        std::map<std::string, Variant> kvs;
        kvs["str"] = Variant(Variant::TYPE_STR, "str1", 4);
        kvs["bin"] = Variant(Variant::TYPE_BIN, "bin1", 4);

        if (mongodb_engine_->Update(db_name, coll_name, filter, kvs, mongocxx::options::update{}) != 0)
        {
            FAIL();
        }
    }
}

void MongodbEngineTest::Test005()
{
    // delete
    {
        // delete的doc不存在时，delete操作仍然会返回成功
        std::map<std::string, Variant> kvs_filter;
        kvs_filter["player_id"] = Variant(2);

        bsoncxx::document::value filter = mongodb_engine_->Kvs2Doc(kvs_filter);

        if (mongodb_engine_->Delete(db_name, coll_name, filter, mongocxx::options::delete_options{}) != 0)
        {
            FAIL();
        }
    }

    {
        std::map<std::string, Variant> kvs_filter;
        kvs_filter["player_id"] = Variant(1);

        bsoncxx::document::value filter = mongodb_engine_->Kvs2Doc(kvs_filter);

        if (mongodb_engine_->Delete(db_name, coll_name, filter, mongocxx::options::delete_options{}) != 0)
        {
            FAIL();
        }
    }
}

void MongodbEngineTest::Test006()
{
    // json
    std::string json(
        "{ \"f32\" : 0.5, \"f64\" : 9.9000000000000003553, \"i32\" : 100, \"i64\" : 9999, \"player_id\" : 2, \"str\" : \"str1\" }");

    if (mongodb_engine_->Insert(db_name, coll_name, mongodb_engine_->Json2Doc(json), mongocxx::options::insert{}) != 0)
    {
        FAIL();
    }

    std::string filter("{ \"player_id\" : 2 }");

    std::vector<bsoncxx::document::view> results;

    if (mongodb_engine_->Find(results, db_name, coll_name, mongodb_engine_->Json2Doc(filter),
                              mongocxx::options::find{}) != 0)
    {
        FAIL();
    }

    for (std::vector<bsoncxx::document::view>::const_iterator it = results.begin(); it != results.end(); ++it)
    {
        std::string json(mongodb_engine_->Doc2Json(*it));
        LOG_TRACE("json: " << json);
    }
}

void MongodbEngineTest::Test007()
{
    // concat

    // part1
    auto doc_part1_builder = bsoncxx::builder::stream::document{};
    bsoncxx::document::value doc_part1 = doc_part1_builder
        << "name" << "MongoDB"
        << "type" << "database"
        << "count" << 1
        << "versions" << bsoncxx::builder::stream::open_array
        << "v3.2" << "v3.0" << "v2.6"
        << close_array
        << "info" << bsoncxx::builder::stream::open_document
        << "x" << 203
        << "y" << 102
        << bsoncxx::builder::stream::close_document
        << bsoncxx::builder::stream::finalize;

    // part2
    // Use builder::stream::array for array building
    auto doc_part2_builder = bsoncxx::builder::stream::document{};

    // Makes { "key" : "value", "subdoc" : { "sub key a" : 1 } }
    bsoncxx::document::value doc_part2 =
        doc_part2_builder << "key" << "value" << "subdoc" << open_document << "sub key a" << 1 << close_document
            << finalize;

    // part3
    std::map<std::string, Variant> kvs;

    kvs["player_id"] = Variant(1);
    kvs["i32"] = Variant(100);
    kvs["i64"] = Variant(INT64_C(9999));
    kvs["f32"] = Variant(0.5f);
    kvs["f64"] = Variant(9.9);
    kvs["str"] = Variant(Variant::TYPE_STR, "str1", 4);
    kvs["bin"] = Variant(Variant::TYPE_BIN, "bin1", 4);

    bsoncxx::document::value doc_part3 = mongodb_engine_->Kvs2Doc(kvs);

    auto doc_builder = bsoncxx::builder::stream::document{};
    doc_builder << bsoncxx::builder::concatenate(doc_part1.view()) << bsoncxx::builder::concatenate(doc_part2.view())
        << bsoncxx::builder::concatenate(doc_part3.view());
    bsoncxx::document::value doc = doc_builder.extract();
    LOG_TRACE(bsoncxx::to_json(doc));

    std::vector<bsoncxx::document::value> doc_parts;
    doc_parts.push_back(doc_part1);
    doc_parts.push_back(doc_part2);
    doc_parts.push_back(doc_part3);

    bsoncxx::document::value doc_ = mongodb_engine_->ConcatDocParts(doc_parts);
    LOG_TRACE(bsoncxx::to_json(doc_));
}

void MongodbEngineTest::Test008()
{
    // array
//
//    {
//        "data": [
//        [1.0, 2.0, 3.0], [4.0, 5.0, 6.0]
//        ]
//    }

    bsoncxx::builder::stream::document doc{};
    auto arr = doc << "data" << open_array;

    for (int i = 0; i < 5; ++i)
    {
        double v1 = i * 3;
        double v2 = i * 3 + 1;
        double v3 = i * 3 + 2;
        arr << open_array << v1 << v2 << v3 << close_array;
    }

    arr << close_array;
    std::cout << bsoncxx::to_json(doc) << std::endl;

    std::string json(
        "{ \"data\" : [ [ 0.0, 1.0, 2.0 ], [ 3.0, 4.0, 5.0 ], [ 6.0, 7.0, 8.0 ], [ 9.0, 10.0, 11.0 ], [ 12.0, 13.0, 14.0 ] ] }");
    LOG_TRACE(mongodb_engine_->Doc2Json(mongodb_engine_->Json2Doc(json)));
}

ADD_TEST_F(MongodbEngineTest, Test001);
ADD_TEST_F(MongodbEngineTest, Test002);
ADD_TEST_F(MongodbEngineTest, Test003);
ADD_TEST_F(MongodbEngineTest, Test004);
ADD_TEST_F(MongodbEngineTest, Test005);
ADD_TEST_F(MongodbEngineTest, Test006);
ADD_TEST_F(MongodbEngineTest, Test007);
ADD_TEST_F(MongodbEngineTest, Test008);
