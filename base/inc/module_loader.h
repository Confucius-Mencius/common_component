/**
 * @file module_loader.h
 * @brief 模块so加载器
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_MODULE_LOADER_H_
#define BASE_INC_MODULE_LOADER_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_ModuleLoader ModuleLoader
 * @{
 */

#include "last_err_msg.h"
#include "module_util.h"

class ModuleLoader
{
public:
    ModuleLoader();
    ~ModuleLoader();

    const char* GetLastErrMsg() const;

    int Load(const char* module_file_path);
    int Unload();

    void* GET_MODULE_INTERFACE(int type = 0);

private:
    void SetOpenFailedErrMsg(const char* module_file_path, const char* err);
    void SetFindSymFailedErrMsg(const char* module_file_path, const char* err);
    void SetCloseFailedErrMsg(const char* module_file_path, const char* err);

private:
    std::string module_file_path_;
    LastErrMsg last_err_msg_;

#if (defined(__linux__))
    void* module_;
#elif (defined(_WIN32) || defined(_WIN64))
    HMODULE module_;
#endif

    GetModuleInterfaceFunc get_module_interface_func_;
};

/** @} Module_ModuleLoader */
/** @} Module_Base */

#endif // BASE_INC_MODULE_LOADER_H_
