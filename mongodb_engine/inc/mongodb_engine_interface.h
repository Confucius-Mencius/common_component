#ifndef MONGODB_ENGINE_INC_MONGODB_ENGINE_INTERFACE_H_
#define MONGODB_ENGINE_INC_MONGODB_ENGINE_INTERFACE_H_

#include "module_interface.h"
#include <string>
#include <map>
#include <mongocxx/options/update.hpp>
#include <mongocxx/options/insert.hpp>
#include "variant.h"
#include "mongocxx/instance.hpp"
#include "mongocxx/pool.hpp"
#include "mongocxx/options/index.hpp"
#include "mongocxx/options/insert.hpp"
#include "mongocxx/options/delete.hpp"
#include "mongocxx/options/update.hpp"
#include "mongocxx/options/find.hpp"

struct MongodbEngineCtx
{
    mongocxx::pool* mongodb_pool;

    MongodbEngineCtx()
    {
        mongodb_pool = NULL;
    }
};

class MongodbEngineInterface : public ModuleInterface
{
public:
    virtual ~MongodbEngineInterface()
    {
    }

    /**
     * ＠brief 全局初始化，在多线程环境下只能被调用一次
     * @param db_conn_str
     * @return
     */
    static std::pair<mongocxx::instance*, mongocxx::pool*> GlobalInitialize(const char* db_conn_str)
    {
        mongocxx::instance* mongodb_instance = new mongocxx::instance(); // This should be done only once.
        if (NULL == mongodb_instance)
        {
            return std::pair<mongocxx::instance*, mongocxx::pool*>(NULL, NULL);
        }

        mongocxx::pool* mongodb_pool = new mongocxx::pool(mongocxx::uri(db_conn_str));
        if (NULL == mongodb_pool)
        {
            delete mongodb_instance;
            return std::pair<mongocxx::instance*, mongocxx::pool*>(NULL, NULL);
        }

        return std::pair<mongocxx::instance*, mongocxx::pool*>(mongodb_instance, mongodb_pool);
    }

    static void GlobalFinalize(std::pair<mongocxx::instance*, mongocxx::pool*>& mongodb_ctx)
    {
        if (mongodb_ctx.second != NULL)
        {
            delete mongodb_ctx.second;
        }

        if (mongodb_ctx.first != NULL)
        {
            delete mongodb_ctx.first;
        }
    }

    virtual bsoncxx::document::value Json2Doc(const std::string& json) = 0;
    virtual std::string Doc2Json(const bsoncxx::document::view& doc) = 0;

    virtual bsoncxx::document::value Kvs2Doc(const std::map<std::string, Variant>& kvs) = 0;
    virtual std::map<std::string, Variant> Doc2Kvs(const bsoncxx::document::view& doc) = 0;

    /**
     * @brief 将几个doc片段连接起来，doc片段可以是通过Json2Doc生成的literal doc，也可以是通过Kvs2Doc生成的包含二进制数据的doc
     * @param doc_parts
     * @return =0表示成功，否则失败
     */
    virtual bsoncxx::document::value ConcatDocParts(const std::vector<bsoncxx::document::value>& doc_parts) = 0;

    virtual bool TryLockColl(const char* db_name, const char* coll_name, const bsoncxx::document::value& key) = 0;
    virtual bool LockColl(const char* db_name, const char* coll_name, const bsoncxx::document::value& key) = 0;
    virtual void UnlockColl(const char* db_name, const char* coll_name, const bsoncxx::document::value& key) = 0;

    /**
     * @brief 在集合上创建索引
     * @param db_name
     * @param coll_name
     * @param index
     * @param options
     * @return  =0表示成功，否则失败
     * @attention 反复调用创建同一个索引多次没有副作用。
     */
    virtual int CreateIndex(const char* db_name, const char* coll_name, const bsoncxx::document::value& index,
                            const mongocxx::options::index& options) = 0;

    /**
     *@brief 插入一条或多条文档
     * @param db_name
     * @param coll_name
     * @param doc
     * @return =0表示成功，否则失败
     * @attention 1,在unique index的集合中第二次插入相同的关键字就失败
     */
    virtual int Insert(const char* db_name, const char* coll_name, const bsoncxx::document::value& doc,
                       const mongocxx::options::insert& options) = 0;
    virtual int Insert(const char* db_name, const char* coll_name,
                       const std::vector<bsoncxx::document::value>& docs,
                       const mongocxx::options::insert& options) = 0;

    /**
     * @brief 删除符合条件的文档
     * @param db_name
     * @param coll_name
     * @param filter
     * @return =0表示成功，否则失败
     * @attention 1,delete的doc不存在时，delete操作仍然会返回成功
     */
    virtual int Delete(const char* db_name, const char* coll_name, const bsoncxx::document::value& filter,
                       const mongocxx::options::delete_options& options) = 0;

    /**
     * @brief 更新文档
     * @param db_name
     * @param coll_name
     * @param filter
     * @param update
     * @param many
     * @return =0表示成功，否则失败
     * @attention 1,如果set的值没有变化，update操作返回成功
     *            2,要更新的字段不存在时，update操作返回成功，且会自动增加该字段和值
     *            3,filter描述的doc不存在时，update操作返回成功，但是不会增加一个新doc
     */
    virtual int Update(const char* db_name, const char* coll_name, const bsoncxx::document::value& filter,
                       const bsoncxx::document::value& update, const mongocxx::options::update& options) = 0;

    /**
     * @brief 更新指定的一批字段，可以包含二进制数据
     * @param db_name
     * @param coll_name
     * @param filter
     * @param kvs
     * @return
     */
    virtual int Update(const char* db_name, const char* coll_name, const bsoncxx::document::value& filter,
                       const std::map<std::string, Variant>& kvs, const mongocxx::options::update& options) = 0;

    /**
     * @brief 查找文档
     * @param docs
     * @param db_name
     * @param coll_name
     * @param filter
     * @return
     */
    virtual int Find(std::vector<bsoncxx::document::view>& docs, const char* db_name, const char* coll_name,
                     const bsoncxx::document::value& filter, const mongocxx::options::find& options) = 0;
};

#endif // MONGODB_ENGINE_INC_MONGODB_ENGINE_INTERFACE_H_
