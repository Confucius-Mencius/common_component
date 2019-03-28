#ifndef CONF_CENTER_INC_CONF_CENTER_INTERFACE_H_
#define CONF_CENTER_INC_CONF_CENTER_INTERFACE_H_

#include "data_types.h"
#include "module_interface.h"

/**
 * xml文件中xpath的格式：
 * 1, node: /conf/db
 * 2, attrib: /conf/db/@name
 */

struct ConfCenterCtx
{
    const char* app_conf_file_path;

    ConfCenterCtx()
    {
        app_conf_file_path = nullptr;
    }
};

class ConfCenterInterface : public ModuleInterface
{
public:
    virtual ~ConfCenterInterface()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////
    // single api
    ////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief GetConf
     * @param conf
     * @param xpath
     * @param with_default 找不到配置时是否返回默认值
     * @param value default value
     * @return =0表示成功，否则失败
     */
    virtual int GetConf(i32& conf, const char* xpath, bool with_default = false, i32 value = 0) = 0;
    virtual int GetConf(i64& conf, const char* xpath, bool with_default = false, i64 value = 0L) = 0;
    virtual int GetConf(f32& conf, const char* xpath, bool with_default = false, f32 value = 0.0f) = 0;
    virtual int GetConf(f64& conf, const char* xpath, bool with_default = false, f64 value = 0.0) = 0;

    // 提供Ｃ风格的接口
    virtual int GetConf(char** conf, const char* xpath, bool with_default = false, const char* value = "") = 0;
    virtual void ReleaseConf(char** conf) = 0;

    ////////////////////////////////////////////////////////////////////////////////
    // group api
    ////////////////////////////////////////////////////////////////////////////////
    /**
     * 获取i32类型的配置组
     * @param conf_array
     * @param n 元素个数。如果找不到配置且with_default为true，则n为1
     * @param xpath
     * @param with_default
     * @param value
     * @return =0表示成功，否则失败
     */
    virtual int GetConf(i32** conf_array, int& n, const char* xpath, bool with_default = false, i32 value = 0) = 0;
    virtual void ReleaseConf(i32** conf_array) = 0;

    virtual int GetConf(i64** conf_array, int& n, const char* xpath, bool with_default = false, i64 value = 0L) = 0;
    virtual void ReleaseConf(i64** conf_array) = 0;

    virtual int GetConf(f32** conf_array, int& n, const char* xpath, bool with_default = false, f32 value = 0.0f) = 0;
    virtual void ReleaseConf(f32** conf_array) = 0;

    virtual int GetConf(f64** conf_array, int& n, const char* xpath, bool with_default = false, f64 value = 0.0) = 0;
    virtual void ReleaseConf(f64** conf_array) = 0;

    virtual int GetConf(char*** conf_array, int& n, const char* xpath, bool with_default = false, const char* value = "") = 0;
    virtual void ReleaseConf(char*** conf_array, int n) = 0;

    /**
     * @brief Reload
     * @param changed 是否有配置变化
     * @return
     */
    virtual int Reload() = 0;
};

#endif // CONF_CENTER_INC_CONF_CENTER_INTERFACE_H_
