#include "mongodb_engine.h"
#include <unistd.h>
#include "log_util.h"
#include "mongocxx/client.hpp"
#include "bsoncxx/exception/exception.hpp"
#include "mongocxx/exception/exception.hpp"
#include "bsoncxx/builder/basic/document.hpp"
#include "bsoncxx/json.hpp"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

namespace mongodb_engine
{
MongodbEngine::MongodbEngine() : mongodb_engine_ctx_()
{
}

MongodbEngine::~MongodbEngine()
{
}

const char* MongodbEngine::GetVersion() const
{
    return NULL;
}

const char* MongodbEngine::GetLastErrMsg() const
{
    return NULL;
}

void MongodbEngine::Release()
{
    delete this;
}

int MongodbEngine::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    mongodb_engine_ctx_ = *((MongodbEngineCtx*) ctx);
    return 0;
}

void MongodbEngine::Finalize()
{
}

int MongodbEngine::Activate()
{
    return 0;
}

void MongodbEngine::Freeze()
{
}

bsoncxx::document::value MongodbEngine::Json2Doc(const std::string& json)
{
    try
    {
        return bsoncxx::from_json(bsoncxx::stdx::string_view(json));
    }
    catch (const bsoncxx::exception& e)
    {
        LOG_ERROR("bsoncxx caught exception: " << e.what());
    }
    catch (const mongocxx::exception& e)
    {
        LOG_ERROR("mongodb caught exception: " << e.what());
    }

    return (bsoncxx::builder::stream::document{} << "" << "" << bsoncxx::builder::stream::finalize);
}

std::string MongodbEngine::Doc2Json(const bsoncxx::document::view& doc)
{
    try
    {
        return bsoncxx::to_json(doc);
    }
    catch (const bsoncxx::exception& e)
    {
        LOG_ERROR("bsoncxx caught exception: " << e.what());
    }
    catch (const mongocxx::exception& e)
    {
        LOG_ERROR("mongodb caught exception: " << e.what());
    }

    return "";
}

bsoncxx::document::value MongodbEngine::Kvs2Doc(const std::map<std::string, Variant>& kvs)
{
    bsoncxx::builder::basic::document basic_builder{};

    try
    {
        for (std::map<std::string, Variant>::const_iterator it = kvs.begin(); it != kvs.end(); ++it)
        {
            switch (it->second.GetType())
            {
                case Variant::TYPE_I32:
                {
                    bsoncxx::types::b_int32 i32_data;
                    i32_data.value = it->second.GetValue(Type2Type<i32>());
                    basic_builder.append(kvp(it->first, i32_data));
                }
                    break;

                case Variant::TYPE_I64:
                {
                    bsoncxx::types::b_int64 i64_data;
                    i64_data.value = it->second.GetValue(Type2Type<i64>());
                    basic_builder.append(kvp(it->first, i64_data));
                }
                    break;

                case Variant::TYPE_F32:
                {
                    bsoncxx::types::b_double f64_data;
                    f64_data.value = it->second.GetValue(Type2Type<f32>());
                    basic_builder.append(kvp(it->first, f64_data));
                }
                    break;

                case Variant::TYPE_F64:
                {
                    bsoncxx::types::b_double f64_data;
                    f64_data.value = it->second.GetValue(Type2Type<f64>());
                    basic_builder.append(kvp(it->first, f64_data));
                }
                    break;

                case Variant::TYPE_STR:
                {
                    const Variant::Chunk& chunk = it->second.GetValue(Type2Type<const char*>());
                    bsoncxx::types::b_utf8 utf8_str_data(chunk.data);
                    basic_builder.append(kvp(it->first, utf8_str_data));
                }
                    break;

                case Variant::TYPE_BIN:
                {
                    const Variant::Chunk& chunk = it->second.GetValue(Type2Type<const char*>());

                    bsoncxx::types::b_binary bin_data;
                    bin_data.bytes = (const uint8_t*) chunk.data;
                    bin_data.size = chunk.len;

                    basic_builder.append(kvp(it->first, bin_data));
                }
                    break;

                default:
                    break;
            }
        }
    }
    catch (const bsoncxx::exception& e)
    {
        LOG_ERROR("bsoncxx caught exception: " << e.what());
    }
    catch (const mongocxx::exception& e)
    {
        LOG_ERROR("mongodb caught exception: " << e.what());
    }

    return basic_builder.extract();
}

std::map<std::string, Variant> MongodbEngine::Doc2Kvs(const bsoncxx::document::view& doc)
{
    std::map<std::string, Variant> result;

    try
    {
        for (bsoncxx::document::view::const_iterator it = doc.begin(); it != doc.end(); ++it)
        {
            const bsoncxx::document::element& element = *it;

            switch (element.type())
            {
                case bsoncxx::type::k_int32:
                {
                    result[element.key().to_string()] = element.get_int32().value;
                }
                    break;

                case bsoncxx::type::k_int64:
                {
                    result[element.key().to_string()] = element.get_int64().value;
                }
                    break;

                case bsoncxx::type::k_double:
                {
                    result[element.key().to_string()] = element.get_double().value;
                }
                    break;

                case bsoncxx::type::k_utf8:
                {
                    result[element.key().to_string()] = Variant(Variant::TYPE_STR,
                                                                element.get_utf8().value.to_string().c_str(),
                                                                element.get_utf8().value.length());
                }
                    break;

                case bsoncxx::type::k_binary:
                {
                    result[element.key().to_string()] = Variant(Variant::TYPE_BIN,
                                                                (const char*) element.get_binary().bytes,
                                                                element.get_binary().size);
                }
                    break;

                default:
                {
                }
                    break;
            }
        }
    }
    catch (const bsoncxx::exception& e)
    {
        LOG_ERROR("bsoncxx caught exception: " << e.what());
    }
    catch (const mongocxx::exception& e)
    {
        LOG_ERROR("mongodb caught exception: " << e.what());
    }

    return result;
}

bsoncxx::document::value MongodbEngine::ConcatDocParts(const std::vector<bsoncxx::document::value>& doc_parts)
{
    auto doc_builder = bsoncxx::builder::stream::document{};

    try
    {
        for (std::vector<bsoncxx::document::value>::const_iterator it = doc_parts.begin(); it != doc_parts.end(); ++it)
        {
            doc_builder << bsoncxx::builder::concatenate((*it).view());
        }

        return doc_builder.extract();
    }
    catch (const bsoncxx::exception& e)
    {
        LOG_ERROR("bsoncxx caught exception: " << e.what());

    }
    catch (const mongocxx::exception& e)
    {
        LOG_ERROR("mongodb caught exception: " << e.what());
    }

    return doc_builder.extract();
}

bool MongodbEngine::TryLockColl(const char* db_name, const char* coll_name, const bsoncxx::document::value& key)
{
    std::string lock_coll_name;
    lock_coll_name.append(coll_name);
    lock_coll_name.append("_lock");

    try
    {
        mongocxx::pool::entry db_client = mongodb_engine_ctx_.mongodb_pool->acquire();
        mongocxx::database db = db_client->database(db_name);
        mongocxx::collection coll = db.collection(lock_coll_name);

        bsoncxx::stdx::optional<mongocxx::result::insert_one> insert_one_result = coll.insert_one(key.view());
        return true;
    }
    catch (const bsoncxx::exception& e)
    {
        LOG_ERROR("bsoncxx caught exception: " << e.what());

    }
    catch (const mongocxx::exception& e)
    {
        LOG_ERROR("mongodb caught exception: " << e.what());
    }

    return false;
}

bool MongodbEngine::LockColl(const char* db_name, const char* coll_name, const bsoncxx::document::value& key)
{
    while (!TryLockColl(db_name, coll_name, key))
    {
        usleep(1);
    }

    return true;
}

void MongodbEngine::UnlockColl(const char* db_name, const char* coll_name, const bsoncxx::document::value& key)
{
    std::string lock_coll_name;
    lock_coll_name.append(coll_name);
    lock_coll_name.append("_lock");

    try
    {
        mongocxx::pool::entry db_client = mongodb_engine_ctx_.mongodb_pool->acquire();
        mongocxx::database db = db_client->database(db_name);
        mongocxx::collection coll = db.collection(lock_coll_name);

        mongocxx::stdx::optional<mongocxx::result::delete_result> result = coll.delete_one(key.view());
        return;
    }
    catch (const bsoncxx::exception& e)
    {
        LOG_ERROR("bsoncxx caught exception: " << e.what());
    }
    catch (const mongocxx::exception& e)
    {
        LOG_ERROR("mongodb caught exception: " << e.what());
    }
}

// bsoncxx::document::value index_specification = bsoncxx::builder::stream::document{} << "i" << 1 << finalize;
int MongodbEngine::CreateIndex(const char* db_name, const char* coll_name, const bsoncxx::document::value& index,
                               const mongocxx::options::index& options)
{
    try
    {
        mongocxx::pool::entry db_client = mongodb_engine_ctx_.mongodb_pool->acquire();
        mongocxx::database db = db_client->database(db_name);
        mongocxx::collection coll = db.collection(coll_name);
        coll.create_index(std::move(index.view()), options);
        return 0;
    }
    catch (const bsoncxx::exception& e)
    {
        LOG_ERROR("bsoncxx caught exception: " << e.what());
    }
    catch (const mongocxx::exception& e)
    {
        LOG_ERROR("mongodb caught exception: " << e.what());
    }

    return -1;
}

int MongodbEngine::Insert(const char* db_name, const char* coll_name, const bsoncxx::document::value& doc,
                          const mongocxx::options::insert& options)
{
    try
    {
        mongocxx::pool::entry db_client = mongodb_engine_ctx_.mongodb_pool->acquire();
        mongocxx::database db = db_client->database(db_name);
        mongocxx::collection coll = db.collection(coll_name);

        bsoncxx::stdx::optional<mongocxx::result::insert_one> result = coll.insert_one(doc.view(), options);
        if (result)
        {
            const bsoncxx::types::value& id = result.value().inserted_id();
            LOG_TRACE("inserted id: " << id.get_oid().value.to_string());
        }

        return 0;
    }
    catch (const bsoncxx::exception& e)
    {
        LOG_ERROR("bsoncxx caught exception: " << e.what());
    }
    catch (const mongocxx::exception& e)
    {
        LOG_ERROR("mongodb caught exception: " << e.what());
    }

    return -1;
}

/*
auto builder = bsoncxx::builder::stream::document{};
bsoncxx::document::value doc_value = builder
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
 */
int MongodbEngine::Insert(const char* db_name, const char* coll_name, const std::vector<bsoncxx::document::value>& docs,
                          const mongocxx::options::insert& options)
{
    if (0 == docs.size())
    {
        return -1;
    }

    try
    {
        mongocxx::pool::entry db_client = mongodb_engine_ctx_.mongodb_pool->acquire();
        mongocxx::database db = db_client->database(db_name);
        mongocxx::collection coll = db.collection(coll_name);

        if (1 == docs.size())
        {
            bsoncxx::stdx::optional<mongocxx::result::insert_one> result = coll.insert_one(docs[0].view(), options);
            if (result)
            {
                const bsoncxx::types::value& id = result.value().inserted_id();
                LOG_TRACE("inserted id: " << id.get_oid().value.to_string());
            }
        }
        else
        {
            bsoncxx::stdx::optional<mongocxx::result::insert_many> result = coll.insert_many(docs, options);
            if (result)
            {
                mongocxx::result::insert_many::id_map id_map = result.value().inserted_ids();
                for (mongocxx::result::insert_many::id_map::const_iterator it = id_map.begin();
                     it != id_map.end(); ++it)
                {
                    LOG_TRACE("inserted id: " << it->second.get_oid().value.to_string());
                }
            }
        }

        return 0;
    }
    catch (const bsoncxx::exception& e)
    {
        LOG_ERROR("bsoncxx caught exception: " << e.what());
    }
    catch (const mongocxx::exception& e)
    {
        LOG_ERROR("mongodb caught exception: " << e.what());
    }

    return -1;
}

int MongodbEngine::Delete(const char* db_name, const char* coll_name, const bsoncxx::document::value& filter,
                          const mongocxx::options::delete_options& options)
{
    try
    {
        mongocxx::pool::entry db_client = mongodb_engine_ctx_.mongodb_pool->acquire();
        mongocxx::database db = db_client->database(db_name);
        mongocxx::collection coll = db.collection(coll_name);

//        if (many)
//        {
        bsoncxx::stdx::optional<mongocxx::result::delete_result> result = coll.delete_many(filter.view(), options);
        if (result)
        {
            LOG_TRACE("deleted doc count: " << result.value().deleted_count());
        }
//        }
//        else
//        {
//            mongocxx::stdx::optional<mongocxx::result::delete_result> result = coll.delete_one(filter.view());
//            int32_t deleted_count = result.value().deleted_count();
//        }

        return 0;
    }
    catch (const bsoncxx::exception& e)
    {
        LOG_ERROR("bsoncxx caught exception: " << e.what());
    }
    catch (const mongocxx::exception& e)
    {
        LOG_ERROR("mongodb caught exception: " << e.what());
    }

    return -1;
}

int MongodbEngine::Update(const char* db_name, const char* coll_name, const bsoncxx::document::value& filter,
                          const bsoncxx::document::value& update, const mongocxx::options::update& options)
{
    try
    {
        mongocxx::pool::entry db_client = mongodb_engine_ctx_.mongodb_pool->acquire();
        mongocxx::database db = db_client->database(db_name);
        mongocxx::collection coll = db.collection(coll_name);

//        if (many)
//        {
        bsoncxx::stdx::optional<mongocxx::result::update> result = coll.update_many(filter.view(), update.view(),
                                                                                    options);
        if (result)
        {
            if (0 == result.value().matched_count())
            {
                LOG_ERROR("no doc matched the filter: " << bsoncxx::to_json(filter));
                return -1;
            }

            LOG_TRACE("modified doc count: " << result.value().modified_count());
        }

        return 0;
//        }
//        else
//        {
//            bsoncxx::stdx::optional<mongocxx::result::update> result = coll.update_one(filter.view(), update.view());
//        }
    }
    catch (const bsoncxx::exception& e)
    {
        LOG_ERROR("bsoncxx caught exception: " << e.what());
    }
    catch (const mongocxx::exception& e)
    {
        LOG_ERROR("mongodb caught exception: " << e.what());
    }

    return -1;
}

int MongodbEngine::Update(const char* db_name, const char* coll_name, const bsoncxx::document::value& filter,
                          const std::map<std::string, Variant>& kvs, const mongocxx::options::update& options)
{
    try
    {
        bsoncxx::document::value doc = Kvs2Doc(kvs);
        bsoncxx::document::value update = document{} << "$set" << doc << finalize;

        return Update(db_name, coll_name, filter, update, options);
    }
    catch (const bsoncxx::exception& e)
    {
        LOG_ERROR("bsoncxx caught exception: " << e.what());
    }
    catch (const mongocxx::exception& e)
    {
        LOG_ERROR("mongodb caught exception: " << e.what());
    }

    return -1;
}

int MongodbEngine::Find(std::vector<bsoncxx::document::view>& docs, const char* db_name, const char* coll_name,
                        const bsoncxx::document::value& filter, const mongocxx::options::find& options)
{
    try
    {
        mongocxx::pool::entry db_client = mongodb_engine_ctx_.mongodb_pool->acquire();
        mongocxx::database db = db_client->database(db_name);
        mongocxx::collection coll = db.collection(coll_name);

//        if (many)
//        {
        mongocxx::cursor cursor = coll.find(filter.view(), options);
        for (auto doc : cursor)
        {
//                LOG_TRACE(bsoncxx::to_json(doc));
            docs.push_back(doc);
        }

        return 0;
//        }
//        else
//        {
//            bsoncxx::stdx::optional<bsoncxx::document::value> result = coll.find_one(filter.view());
//            if (result)
//            {
//                LOG_TRACE(bsoncxx::to_json(*result));
//                results.push_back(*result); // TODO 传不出去？？
//            }
//        }
    }
    catch (const bsoncxx::exception& e)
    {
        LOG_ERROR("bsoncxx caught exception: " << e.what());
    }
    catch (const mongocxx::exception& e)
    {
        LOG_ERROR("mongodb caught exception: " << e.what());
    }

    return -1;
}

//int MongodbEngine::GetFile(char** file_content, size_t& file_len, const char* gridfs_db_name,
//                           const char* gridfs_file_name)
//{
//    char* buf = NULL;
//
//    try
//    {
//        mongo::GridFS gridfs(*db_client, gridfs_db_name);
//        mongo::GridFile gridfs_file = gridfs.findFileByName(gridfs_file_name);
//
//        if (!gridfs_file.exists())
//        {
//            LOG_ERROR("gridfs file " << gridfs_file_name << " not exist in database " << gridfs_db_name);
//            return -1;
//        }
//
//        file_len = gridfs_file.getContentLength();
//
//        LOG_TRACE("gridfs file len: " << file_len);
//
//        buf = new char[file_len];
//        if (NULL == buf)
//        {
//            LOG_ERROR("failed to create buf");
//            return -1;
//        }
//
//        size_t offset = 0;
//        int nchunks = gridfs_file.getNumChunks();
//
//        for (int i = 0; i < nchunks; ++i)
//        {
//            mongo::GridFSChunk chunk = gridfs_file.getChunk(i);
//            int chunk_len = chunk.len();
//            const char* data = chunk.data(chunk_len);
//            if (NULL == data)
//            {
//                LOG_WARN("null data in chunk, i: " << i);
//                continue;
//            }
//
//            if (offset >= file_len || (offset + chunk_len) > file_len)
//            {
//                LOG_ERROR("invalid chunk, i: " << i << ", offset: " << offset << ", chunk len: " << chunk_len
//                              << ", gridfs file len: " << file_len);
//
//                if (buf != NULL)
//                {
//                    delete[] buf;
//                    buf = NULL;
//                }
//
//                return -1;
//            }
//
//            memcpy(buf + offset, data, chunk_len);
//            offset += chunk_len;
//        }
//    }
//    catch (const mongo::DBException& e)
//    {
//        LOG_ERROR("mongodb caught exception: " << e.what());
//
//        if (buf != NULL)
//        {
//            delete[] buf;
//            buf = NULL;
//        }
//
//        return -1;
//    }
//
//    *file_content = buf;
//    return 0;
//}
//
//void MongodbEngine::ReleaseFileContent(char** file_content)
//{
//    if (NULL == file_content || NULL == *file_content)
//    {
//        return;
//    }
//
//    delete[] (*file_content);
//    (*file_content) = NULL;
//}
//
//int MongodbEngine::SetFile(const char* gridfs_db_name, const char* gridfs_file_name, const char* file_content,
//                           size_t file_len)
//{
//    try
//    {
//        mongo::GridFS gridfs(*db_client, gridfs_db_name);
//        gridfs.removeFile(gridfs_file_name); // 如果不删除的话,同名文件会越来越多
//        mongo::BSONObj f = gridfs.storeFile(file_content, file_len, gridfs_file_name);
//    }
//    catch (const mongo::DBException& e)
//    {
//        LOG_ERROR("mongodb caught exception: " << e.what());
//        return -1;
//    }
//
//    return 0;
//}
//
//mongo::BSONObj MongodbEngine::BuildBSONObj(const KeyValMap& kvs)
//{
//    mongo::BSONObjBuilder builder;
//
//    try
//    {
//        for (KeyValMap::const_iterator it = kvs.begin(); it != kvs.end(); ++it)
//        {
//            switch (it->second.GetType())
//            {
//                case Variant::TYPE_I32:
//                {
//                    builder.append(it->first, it->second.GetValue(Type2Type<i32>()));
//                }
//                    break;
//
//                case Variant::TYPE_I64:
//                {
//                    builder.append(it->first, (long long) it->second.GetValue(Type2Type<i64>()));
//                }
//                    break;
//
//                case Variant::TYPE_F32:
//                {
//                    builder.append(it->first, it->second.GetValue(Type2Type<f32>()));
//                }
//                    break;
//
//                case Variant::TYPE_F64:
//                {
//                    builder.append(it->first, it->second.GetValue(Type2Type<f64>()));
//                }
//                    break;
//
//                case Variant::TYPE_STR:
//                {
//                    builder.append(it->first, it->second.GetValue(Type2Type<const char*>()));
//                }
//                    break;
//
//                default:
//                    break;
//            }
//        }
//    }
//    catch (const mongo::DBException& e)
//    {
//        LOG_ERROR("mongodb caught exception: " << e.what());
//        return mongo::BSONObj();
//    }
//
//    return builder.obj();
//}
//
//int MongodbEngine::DumpArray(BSONObjVec& v, const BSONElementVec& array)
//{
//    try
//    {
//        for (BSONElementVec::const_iterator it = array.begin(); it != array.end(); ++it)
//        {
//            mongo::BSONObj item = (*it).embeddedObject();
//            v.push_back(item);
//        }
//    }
//    catch (const mongo::DBException& e)
//    {
//        LOG_ERROR("mongodb caught exception: " << e.what());
//        return -1;
//    }
//
//    return 0;
//}
}
