#include "conf_center.h"
#include "str_util.h"
#include "version.h"

namespace conf_center
{
ConfCenter::ConfCenter() : last_err_msg_(), conf_center_ctx_(), mutex_(), app_conf_file_stat_(), i64_value_group_map_(),
    f64_value_group_map_(), str_value_group_map_()
{
    xml_doc_ptr_ = NULL;
}

ConfCenter::~ConfCenter()
{
}

const char* ConfCenter::GetVersion() const
{
    return CONF_CENTER_VERSION;
}

const char* ConfCenter::GetLastErrMsg() const
{
    return last_err_msg_.What();
}

void ConfCenter::Release()
{
    delete this;
}

int ConfCenter::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    conf_center_ctx_ = *((ConfCenterCtx*) ctx);
    return Load(conf_center_ctx_.app_conf_file_path);
}

void ConfCenter::Finalize()
{
    if (xml_doc_ptr_ != NULL)
    {
        xmlFreeDoc(xml_doc_ptr_);
        xml_doc_ptr_ = NULL;
    }

    xmlCleanupParser();
}

int ConfCenter::Activate()
{
    return 0;
}

void ConfCenter::Freeze()
{
}

int ConfCenter::GetConf(i32& conf, const char* xpath, bool with_default, i32 value)
{
    if (NULL == xpath)
    {
        return -1;
    }

    Int64Group result;
    GetConf<Int64Group, Int64ValueGroupMap, i64>(result, i64_value_group_map_, xpath, with_default,
            Variant(value), Variant::TYPE_I64);

    if (!with_default && 0 == result.size())
    {
        return -1;
    }

    conf = (i32) result[0];
    return 0;
}

int ConfCenter::GetConf(i64& conf, const char* xpath, bool with_default, i64 value)
{
    if (NULL == xpath)
    {
        return -1;
    }

    Int64Group result;
    GetConf<Int64Group, Int64ValueGroupMap, i64>(result, i64_value_group_map_, xpath, with_default,
            Variant(value), Variant::TYPE_I64);

    if (!with_default && 0 == result.size())
    {
        return -1;
    }

    conf = result[0];
    return 0;
}

int ConfCenter::GetConf(f32& conf, const char* xpath, bool with_default, f32 value)
{
    if (NULL == xpath)
    {
        return -1;
    }

    Float64Group result;
    GetConf<Float64Group, Float64ValueGroupMap, f64>(result, f64_value_group_map_, xpath, with_default,
            Variant(value), Variant::TYPE_F64);

    if (!with_default && 0 == result.size())
    {
        return -1;
    }

    conf = (f32) result[0];
    return 0;
}

int ConfCenter::GetConf(f64& conf, const char* xpath, bool with_default, f64 value)
{
    if (NULL == xpath)
    {
        return -1;
    }

    Float64Group result;
    GetConf<Float64Group, Float64ValueGroupMap, f64>(result, f64_value_group_map_, xpath, with_default,
            Variant(value), Variant::TYPE_F64);

    if (!with_default && 0 == result.size())
    {
        return -1;
    }

    conf = result[0];
    return 0;
}

int ConfCenter::GetConf(char** conf, const char* xpath, bool with_default, const char* value)
{
    if (NULL == conf || NULL == xpath)
    {
        return -1;
    }

    if (with_default && NULL == value)
    {
        return -1;
    }

    StrGroup result;
    GetConf(result, str_value_group_map_, xpath, with_default, Variant(Variant::TYPE_STR, value, strlen(value)));

    if (!with_default && 0 == result.size())
    {
        return -1;
    }

    const int size = (int) result[0].length() + 1;

    *conf = new char[size];
    if (NULL == (*conf))
    {
        const int err = errno;
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to alloc memory, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    StrCpy(*conf, size, result[0].c_str());
    (*conf)[size - 1] = '\0';

    return 0;
}

void ConfCenter::ReleaseConf(char** conf)
{
    if (NULL == conf)
    {
        return;
    }

    if ((*conf) != NULL)
    {
        delete[] (*conf);
        *conf = NULL;
    }
}

int ConfCenter::GetConf(i32** conf_array, int& n, const char* xpath, bool with_default, i32 value)
{
    if (NULL == conf_array || NULL == xpath)
    {
        return -1;
    }

    Int64Group result;
    GetConf<Int64Group, Int64ValueGroupMap, i64>(result, i64_value_group_map_, xpath, with_default,
            Variant(value), Variant::TYPE_I64);

    n = (int) result.size();

    if (!with_default && 0 == n)
    {
        return -1;
    }

    *conf_array = new i32[n];
    if (NULL == (*conf_array))
    {
        const int err = errno;
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to alloc memory, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    for (int i = 0; i < n; ++i)
    {
        (*conf_array)[i] = (i32) result[i];
    }

    return 0;
}

void ConfCenter::ReleaseConf(i32** conf_array)
{
    if (NULL == conf_array)
    {
        return;
    }

    if ((*conf_array) != NULL)
    {
        delete[] (*conf_array);
        *conf_array = NULL;
    }
}

int ConfCenter::GetConf(i64** conf_array, int& n, const char* xpath, bool with_default, i64 value)
{
    if (NULL == conf_array || NULL == xpath)
    {
        return -1;
    }

    Int64Group result;
    GetConf<Int64Group, Int64ValueGroupMap, i64>(result, i64_value_group_map_, xpath, with_default,
            Variant(value), Variant::TYPE_I64);

    n = (int) result.size();

    if (!with_default && 0 == n)
    {
        return -1;
    }

    *conf_array = new i64[n];
    if (NULL == (*conf_array))
    {
        const int err = errno;
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to alloc memory, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    for (int i = 0; i < n; ++i)
    {
        (*conf_array)[i] = result[i];
    }

    return 0;
}

void ConfCenter::ReleaseConf(i64** conf_array)
{
    if (NULL == conf_array)
    {
        return;
    }

    if ((*conf_array) != NULL)
    {
        delete[] (*conf_array);
        *conf_array = NULL;
    }
}

int ConfCenter::GetConf(f32** conf_array, int& n, const char* xpath, bool with_default, f32 value)
{
    if (NULL == conf_array || NULL == xpath)
    {
        return -1;
    }

    Float64Group result;
    GetConf<Float64Group, Float64ValueGroupMap, f64>(result, f64_value_group_map_, xpath, with_default,
            Variant(value), Variant::TYPE_F64);

    n = (int) result.size();

    if (!with_default && 0 == n)
    {
        return -1;
    }

    *conf_array = new f32[n];
    if (NULL == (*conf_array))
    {
        const int err = errno;
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to alloc memory, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    for (int i = 0; i < n; ++i)
    {
        (*conf_array)[i] = (f32) result[i];
    }

    return 0;
}

void ConfCenter::ReleaseConf(f32** conf_array)
{
    if (NULL == conf_array)
    {
        return;
    }

    if ((*conf_array) != NULL)
    {
        delete[] (*conf_array);
        *conf_array = NULL;
    }
}

int ConfCenter::GetConf(f64** conf_array, int& n, const char* xpath, bool with_default, f64 value)
{
    if (NULL == conf_array || NULL == xpath)
    {
        return -1;
    }

    Float64Group result;
    GetConf<Float64Group, Float64ValueGroupMap, f64>(result, f64_value_group_map_, xpath, with_default,
            Variant(value), Variant::TYPE_F64);

    n = (int) result.size();

    if (!with_default && 0 == n)
    {
        return -1;
    }

    *conf_array = new f64[n];
    if (NULL == (*conf_array))
    {
        const int err = errno;
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to alloc memory, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    for (int i = 0; i < n; ++i)
    {
        (*conf_array)[i] = result[i];
    }

    return 0;
}

void ConfCenter::ReleaseConf(f64** conf_array)
{
    if (NULL == conf_array)
    {
        return;
    }

    if ((*conf_array) != NULL)
    {
        delete[] (*conf_array);
        *conf_array = NULL;
    }
}

int ConfCenter::GetConf(char*** conf_array, int& n, const char* xpath, bool with_default, const char* value)
{
    if (NULL == conf_array || NULL == xpath)
    {
        return -1;
    }

    if (with_default && NULL == value)
    {
        return -1;
    }

    StrGroup result;
    GetConf(result, str_value_group_map_, xpath, with_default, Variant(Variant::TYPE_STR, value, strlen(value)));

    if (!with_default && 0 == result.size())
    {
        return -1;
    }

    n = (int) result.size();

    *conf_array = new char* [n];
    if (NULL == (*conf_array))
    {
        const int err = errno;
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to alloc memory, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    memset(*conf_array, 0, sizeof(char*) * n);

    for (int i = 0; i < n; ++i)
    {
        const int size = (int) result[i].length() + 1;

        (*conf_array)[i] = new char[size];
        if (NULL == ((*conf_array)[i]))
        {
            const int err = errno;
            SET_LAST_ERR_MSG(&last_err_msg_, "failed to alloc memory, errno: " << err
                             << ", err msg: " << strerror(err));
            ReleaseConf(conf_array, n);
            return -1;
        }

        StrCpy((*conf_array)[i], size, result[i].c_str());
        ((*conf_array)[i])[size - 1] = '\0';
    }

    return 0;
}

void ConfCenter::ReleaseConf(char*** conf_array, int n)
{
    if (NULL == conf_array || n < 1)
    {
        return;
    }

    if ((*conf_array) != NULL)
    {
        for (int i = 0; i < n; ++i)
        {
            if ((*conf_array)[i] != NULL)
            {
                delete[] (*conf_array)[i];
                (*conf_array)[i] = NULL;
            }
        }

        delete[] (*conf_array);
        *conf_array = NULL;
    }
}

int ConfCenter::Reload(bool& changed)
{
    FileStat file_stat;

    if (0 == GetFileStat(file_stat, conf_center_ctx_.app_conf_file_path))
    {
        if (file_stat.Equals(app_conf_file_stat_))
        {
            changed = false;
            return 0;
        }

        changed = true;
        app_conf_file_stat_ = file_stat;
        return Load(conf_center_ctx_.app_conf_file_path);
    }

    return -1;
}

int ConfCenter::Load(const char* conf_file_path)
{
    // 读取xml文件到内存中
    if (!FileExist(conf_file_path))
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "app conf file " << conf_file_path << " not exist");
        return -1;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    if (xml_doc_ptr_ != NULL)
    {
        xmlFreeDoc(xml_doc_ptr_);
        xml_doc_ptr_ = NULL;
    }

    xmlCleanupParser();

    i64_value_group_map_.clear();
    f64_value_group_map_.clear();
    str_value_group_map_.clear();

    int ret = -1;

    do
    {
        xml_doc_ptr_ = xmlReadFile(conf_file_path, "UTF-8", XML_PARSE_RECOVER);
        if (NULL == xml_doc_ptr_)
        {
            SET_LAST_ERR_MSG(&last_err_msg_, "failed to read xml file " << conf_file_path);
            break;
        }

        xmlErrorPtr xml_err = xmlGetLastError();
        if (xml_err != NULL)
        {
            SET_LAST_ERR_MSG(&last_err_msg_, "xml error, code: " << xml_err->code << ", msg: " << xml_err->message);
            break;
        }

        xmlNodePtr root_node = xmlDocGetRootElement(xml_doc_ptr_);
        if (NULL == root_node)
        {
            SET_LAST_ERR_MSG(&last_err_msg_, "no root node found in xml file " << conf_file_path);
            break;
        }

        if (GetFileStat(app_conf_file_stat_, conf_file_path) != 0)
        {
            SET_LAST_ERR_MSG(&last_err_msg_, "failed to get file stat: " << conf_file_path);
            break;
        }

        ret = 0;
    } while (0);

    return ret;
}

int ConfCenter::GetEntries(const char* entry_xpath, VariantVec& entries, Variant::Type data_type)
{
    if (NULL == entry_xpath)
    {
        return -1;
    }

    if (data_type < Variant::TYPE_MIN || data_type >= Variant::TYPE_MAX)
    {
        return -1;
    }

    XmlCharVec note_content_vec;

    int ret = GetNodeContent(note_content_vec, BAD_CAST (entry_xpath));
    if (ret != 0)
    {
        return ret;
    }

    if (0 == note_content_vec.size())
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to find entry of xpath " << entry_xpath);
        return -1;
    }

    Variant entry;

    for (XmlCharVec::iterator it = note_content_vec.begin(); it != note_content_vec.end(); ++it)
    {
        xmlChar* node_content = *it;
        if (NULL == node_content)
        {
            continue;
        }

        switch (data_type)
        {
            case Variant::TYPE_I64:
            {
                entry = atol((const char*) node_content);
            }
            break;

            case Variant::TYPE_F64:
            {
                entry = atof((const char*) node_content);
            }
            break;

            case Variant::TYPE_STR:
            {
                entry = Variant(Variant::TYPE_STR, (const char*) node_content, xmlStrlen(node_content));
            }
            break;

            default:
            {
                xmlFree(node_content);
                *it = NULL;
                continue;
            }
            break;
        }

        entries.push_back(entry);

        xmlFree(node_content);
        *it = NULL;
    }

    return 0;
}

int ConfCenter::GetNodeContent(XmlCharVec& node_content_vec, const xmlChar* xpath) const
{
    xmlXPathObjectPtr xpath_obj_ptr = GetObjPtr(xml_doc_ptr_, xpath);
    if (NULL == xpath_obj_ptr)
    {
        return -1;
    }

    xmlNodeSetPtr node_set_ptr = xpath_obj_ptr->nodesetval;
    xmlChar* node_content = NULL;

    for (int i = 0; i < node_set_ptr->nodeNr; ++i)
    {
        xmlNodePtr node_ptr = node_set_ptr->nodeTab[i];
        node_ptr = node_ptr->xmlChildrenNode;

        while (node_ptr)
        {
            node_content = xmlNodeGetContent(node_ptr);
            if (node_content != NULL)
            {
                node_content_vec.push_back(node_content);
            }

            node_ptr = node_ptr->next;
        }
    }

    xmlXPathFreeObject(xpath_obj_ptr);
    return 0;
}

xmlXPathObjectPtr ConfCenter::GetObjPtr(xmlDocPtr xml_doc_ptr, const xmlChar* xpath) const
{
    xmlXPathContextPtr xpath_ctx_ptr = xmlXPathNewContext(xml_doc_ptr);
    if (NULL == xpath_ctx_ptr)
    {
        return NULL;
    }

    xmlXPathObjectPtr xpath_obj_ptr = xmlXPathEvalExpression(xpath, xpath_ctx_ptr);
    xmlXPathFreeContext(xpath_ctx_ptr);

    if (NULL == xpath_obj_ptr)
    {
        return NULL;
    }

    if (xmlXPathNodeSetIsEmpty(xpath_obj_ptr->nodesetval))
    {
        xmlXPathFreeObject(xpath_obj_ptr);
        return NULL;
    }

    return xpath_obj_ptr;
}

// 这个模板函数的作用个类似于宏，是为了减少重复代码
template<typename ValueGroupType, typename ValueGroupMapType, typename DataType>
void ConfCenter::GetConf(ValueGroupType& result, ValueGroupMapType& value_group_map, const char* xpath,
                         bool with_default, Variant value, Variant::Type data_type)
{
    std::lock_guard<std::mutex> lock(mutex_);

    typename ValueGroupMapType::const_iterator it = value_group_map.find(xpath);
    if (it != value_group_map.end())
    {
        // 内存中有则从内存中读
        const ValueGroupType& value_group = it->second;
        result = value_group;
        return;
    }

    // 从xml文件中解析
    VariantVec entries;

    if (GetEntries(xpath, entries, data_type) != 0 || 0 == entries.size())
    {
        // 解析失败或者找不到
        if (with_default)
        {
            result.push_back(value.GetValue(Type2Type<DataType>()));
        }

        return;
    }

    // 从xml中读到了，放入内存中以便下次使用
    ValueGroupType& value_group = value_group_map[xpath];
    for (VariantVec::const_iterator it_entry = entries.begin(); it_entry != entries.end(); ++it_entry)
    {
        value_group.push_back(it_entry->GetValue(Type2Type<DataType>()));
    }

    result = value_group;
}

void ConfCenter::GetConf(StrGroup& result, ConfCenter::StrValueGroupMap& value_group_map,
                         const char* xpath, bool with_default, Variant value)
{
    std::lock_guard<std::mutex> lock(mutex_);

    typename StrValueGroupMap::const_iterator it = value_group_map.find(xpath);
    if (it != value_group_map.end())
    {
        // 内存中有则从内存中读
        const StrGroup& value_group = it->second;
        result = value_group;
        return;
    }

    // 从xml文件中解析
    VariantVec entries;

    if (GetEntries(xpath, entries, Variant::TYPE_STR) != 0 || 0 == entries.size())
    {
        // 解析失败或者找不到
        if (with_default)
        {
            result.push_back(value.GetValue(Type2Type<const char*>()).data);
        }

        return;
    }

    // 从xml中读到了，放入内存中以便下次使用
    StrGroup& value_group = value_group_map[xpath];
    for (VariantVec::const_iterator it_entry = entries.begin(); it_entry != entries.end(); ++it_entry)
    {
        value_group.push_back(it_entry->GetValue(Type2Type<const char*>()).data);
    }

    result = value_group;
}
}
