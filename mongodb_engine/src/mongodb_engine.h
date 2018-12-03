#ifndef MONGODB_ENGINE_SRC_MONGODB_ENGINE_H_
#define MONGODB_ENGINE_SRC_MONGODB_ENGINE_H_

#include "mongodb_engine_interface.h"

namespace mongodb_engine
{
class MongodbEngine : public MongodbEngineInterface
{
public:
    MongodbEngine();
    virtual ~MongodbEngine();

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// mongodb_engine_ctx_.mongodb_poolEngineInterface /////////////////////////
    bsoncxx::document::value Json2Doc(const std::string& json) override;
    std::string Doc2Json(const bsoncxx::document::view& doc) override;

    bsoncxx::document::value Kvs2Doc(const std::map<std::string, Variant>& kvs) override;
    std::map<std::string, Variant> Doc2Kvs(const bsoncxx::document::view& doc) override;

    bsoncxx::document::value ConcatDocParts(const std::vector<bsoncxx::document::value>& doc_parts) override;

    bool TryLockColl(const char* db_name, const char* coll_name, const bsoncxx::document::value& key) override;
    bool LockColl(const char* db_name, const char* coll_name, const bsoncxx::document::value& key) override;
    void UnlockColl(const char* db_name, const char* coll_name, const bsoncxx::document::value& key) override;

    int CreateIndex(const char* db_name, const char* coll_name, const bsoncxx::document::value& index,
                    const mongocxx::options::index& options) override;

    int Insert(const char* db_name, const char* coll_name, const bsoncxx::document::value& doc,
               const mongocxx::options::insert& options) override;
    int Insert(const char* db_name, const char* coll_name, const std::vector<bsoncxx::document::value>& docs,
               const mongocxx::options::insert& options) override;

    int Delete(const char* db_name, const char* coll_name, const bsoncxx::document::value& filter,
               const mongocxx::options::delete_options& options) override;

    int Update(const char* db_name, const char* coll_name, const bsoncxx::document::value& filter,
               const bsoncxx::document::value& update, const mongocxx::options::update& options) override;
    int Update(const char* db_name, const char* coll_name, const bsoncxx::document::value& filter,
               const std::map<std::string, Variant>& kvs, const mongocxx::options::update& options) override;

    int Find(std::vector<bsoncxx::document::view>& docs, const char* db_name, const char* coll_name,
             const bsoncxx::document::value& filter, const mongocxx::options::find& options) override;

//    virtual int GetFile(char** file_content, size_t& file_len, const char* gridfs_db_name,
//                        const char* gridfs_file_name);
//    virtual void ReleaseFileContent(char** file_content);
//    virtual int SetFile(const char* gridfs_db_name, const char* gridfs_file_name, const char* file_content,
//                        size_t file_len);

//    virtual mongo::BSONObj BuildBSONObj(const KeyValMap& kvs);

    // std::vector<mongo::BSONElement> elements = obj[FIELD_XIAOHAO_LIST].Array();
    // doc[FIELD_XIAOHAO_UID].str()
//    virtual int DumpArray(BSONObjVec& v, const BSONElementVec& array);

private:
    MongodbEngineCtx mongodb_engine_ctx_;
};
}

#endif // MONGODB_ENGINE_SRC_MONGODB_ENGINE_H_
