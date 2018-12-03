#ifndef CONF_CENTER_SRC_CONF_CENTER_H_
#define CONF_CENTER_SRC_CONF_CENTER_H_

#include <map>
#include <mutex>
#include <vector>
#include <libxml/xpath.h>
#include "conf_center_interface.h"
#include "file_util.h"
#include "last_err_msg.h"
#include "variant.h"

namespace conf_center
{
class ConfCenter : public ConfCenterInterface
{
public:
    ConfCenter();
    virtual ~ConfCenter();

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// ConfCenterInterface /////////////////////////
    int GetConf(i32& conf, const char* xpath, bool with_default, i32 value) override;
    int GetConf(i64& conf, const char* xpath, bool with_default, i64 value) override;
    int GetConf(f32& conf, const char* xpath, bool with_default, f32 value) override;
    int GetConf(f64& conf, const char* xpath, bool with_default, f64 value) override;

    int GetConf(char** conf, const char* xpath, bool with_default, const char* value) override;
    void ReleaseConf(char** conf) override;

    int GetConf(i32** conf_array, int& n, const char* xpath, bool with_default, i32 value) override;
    void ReleaseConf(i32** conf_array) override;

    int GetConf(i64** conf_array, int& n, const char* xpath, bool with_default, i64 value) override;
    void ReleaseConf(i64** conf_array) override;

    int GetConf(f32** conf_array, int& n, const char* xpath, bool with_default, f32 value) override;
    void ReleaseConf(f32** conf_array) override;

    int GetConf(f64** conf_array, int& n, const char* xpath, bool with_default, f64 value) override;
    void ReleaseConf(f64** conf_array) override;

    int GetConf(char*** conf_array, int& n, const char* xpath, bool with_default, const char* value) override;
    void ReleaseConf(char*** conf_array, int n) override;

    int Reload(bool& changed) override;

private:
    // 内部只用i64 f64 string三种类型存储
    typedef std::vector<i64> I64ValueGroup;
    typedef std::vector<f64> F64ValueGroup;
    typedef std::vector<std::string> StrValueGroup;

    typedef std::map<std::string, I64ValueGroup> I64ValueGroupMap;
    typedef std::map<std::string, F64ValueGroup> F64ValueGroupMap;
    typedef std::map<std::string, StrValueGroup> StrValueGroupMap;

    typedef std::vector<xmlChar*> XmlCharVec;
    typedef std::vector<Variant> VariantVec;

    int Load(const char* conf_file_path);
    int GetEntries(const char* entry_xpath, VariantVec& entries, Variant::Type data_type);

    // 返回的指针由调用者负责释放
    int GetNodeContent(XmlCharVec& node_content_vec, const xmlChar* xpath) const;

    // 返回的指针由调用者负责释放
    xmlXPathObjectPtr GetObjPtr(xmlDocPtr xml_doc_ptr, const xmlChar* xpath) const;

    template<typename ValueGroupType, typename ValueGroupMapType, typename DataType>
    void GetConf(ValueGroupType& result, ValueGroupMapType& value_group_map, const char* xpath,
                 bool with_default, Variant value, Variant::Type data_type);

    // str
    void GetConf(StrValueGroup& result, StrValueGroupMap& value_group_map, const char* xpath,
                 bool with_default, Variant value);

private:
    LastErrMsg last_err_msg_;
    ConfCenterCtx conf_center_ctx_;

    std::mutex mutex_; // get的时候有更新内存的操作，不能用读写锁

    xmlDocPtr xml_doc_ptr_;
    FileStat file_stat_;

    I64ValueGroupMap i64_value_group_map_;
    F64ValueGroupMap f64_value_group_map_;
    StrValueGroupMap str_value_group_map_;
};
}

#endif // CONF_CENTER_SRC_CONF_CENTER_H_
