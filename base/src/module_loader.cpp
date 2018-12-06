#include "module_loader.h"

#if (defined(__linux__))
#include <dlfcn.h>
#endif

#include <string.h>

ModuleLoader::ModuleLoader() : module_file_path_(""), last_err_msg_()
{
    module_ = NULL;
    get_module_interface_func_ = NULL;
}

ModuleLoader::~ModuleLoader()
{
}

const char* ModuleLoader::GetLastErrMsg() const
{
    return last_err_msg_.What();
}

#if (defined(__linux__))
int ModuleLoader::Load(const char* module_file_path)
{
    if (NULL == module_file_path || strlen(module_file_path) < 1)
    {
        return -1;
    }

    module_file_path_ = module_file_path;

    module_ = dlopen(module_file_path, RTLD_NOW);
    if (NULL == module_)
    {
        char* err = dlerror();
        SetOpenFailedErrMsg(module_file_path, err);
        return -1;
    }

    dlerror();
    int ret = -1;

    do
    {
        get_module_interface_func_ = (GetModuleInterfaceFunc) dlsym(module_, GET_MODULE_INTERFACE_NAME);
        if (NULL == get_module_interface_func_)
        {
            char* err = dlerror();
            SetFindSymFailedErrMsg(module_file_path, err);
            break;
        }
        else
        {
            char* err = dlerror();
            if (err != NULL)
            {
                SetFindSymFailedErrMsg(module_file_path, err);
                break;
            }
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        dlclose(module_);
        module_ = NULL;
        get_module_interface_func_ = NULL;
    }

    return ret;
}

int ModuleLoader::Unload()
{
    if (module_ != NULL)
    {
        if (dlclose(module_) != 0)
        {
            char* err = dlerror();
            SetCloseFailedErrMsg(module_file_path_.c_str(), err);
        }

        module_ = NULL;
    }

    get_module_interface_func_ = NULL;
    return 0;
}
#elif (defined(_WIN32) || defined(_WIN64))
int ModuleLoader::Load(const char* module_file_path)
{
    if (NULL == module_file_path || strlen(module_file_path) < 1)
    {
        return -1;
    }

    char* err = "";

    module_ = LoadLibrary(module_file_path);
    if (NULL == module_)
    {
        SetOpenFailedErrMsg(module_file_path, err);
        return -1;
    }

    get_module_interface_func_ = (GetModuleInterfaceFunc) GetProcAddress(module_, GET_MODULE_INTERFACE_NAME);
    if (NULL == get_module_interface_func_)
    {
        SetFindSymFailedErrMsg(module_file_path, err);

        FreeLibrary(module_);
        module_ = NULL;
        get_module_interface_func_ = NULL;

        return -1;
    }

    return 0;
}

int ModuleLoader::Unload()
{
    if (module_ != NULL)
    {
        FreeLibrary(module_);
        module_ = NULL;
    }

    get_module_interface_func_ = NULL;
    return 0;
}
#endif

void* ModuleLoader::GET_MODULE_INTERFACE(int type)
{
    if (NULL == get_module_interface_func_)
    {
        return NULL;
    }

    return get_module_interface_func_(type);
}

void ModuleLoader::SetOpenFailedErrMsg(const char* module_file_path, const char* err)
{
    SET_LAST_ERR_MSG(&last_err_msg_, "failed to open file " << module_file_path << ": " << err);
}

void ModuleLoader::SetFindSymFailedErrMsg(const char* module_file_path, const char* err)
{
    SET_LAST_ERR_MSG(&last_err_msg_, "failed to find symbol " << GET_MODULE_INTERFACE_NAME
                     << " in file " << module_file_path << ": " << err);
}

void ModuleLoader::SetCloseFailedErrMsg(const char* module_file_path, const char* err)
{
    SET_LAST_ERR_MSG(&last_err_msg_, "failed to close module " << module_file_path << ": " << err);
}
