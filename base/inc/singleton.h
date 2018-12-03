/**
 * @file singleton.h
 * @brief 
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_SINGLETON_H_
#define BASE_INC_SINGLETON_H_

/** 
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_Singleton 单件
 * @{
 */

#include "misc_util.h"
#include <pthread.h>

/**
 * @brief 单件类模板
 * @param T 单件的类型，要求有一个无参构造函数
 * @par 示例：
 * @code
 *  // 定义唯一的Log对象
 *  #define LogS Singleton<CLog>::Instance()
 *  LogS->...
 * @endcode
 */
template<typename T>
class Singleton
{
DISALLOW_INSTANCE(Singleton);

public:
    /**
     * @brief 获取单件指针
     * @return 单件指针
     */
    static T* Instance()
    {
        if (pthread_once(&once_control_, Singleton::Init) != 0)
        {
            return NULL;
        }

        return instance_;
    }

private:
    static void Init()
    {
        if (NULL == instance_)
        {
            instance_ = new T();
            ::atexit(Destroy);
        }
    }

    static void Destroy()
    {
        // this typedef is to avoid T is not a complete type
        typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
        T_must_be_complete_type dummy;
        (void) dummy;

        if (instance_ != NULL)
        {
            delete instance_;
            instance_ = NULL;
        }
    }

private:
    static pthread_once_t once_control_;
    static T* volatile instance_;
};

template<typename T>
pthread_once_t Singleton<T>::once_control_ = PTHREAD_ONCE_INIT;

template<typename T>
T* volatile Singleton<T>::instance_ = NULL;

/** @} Module_Singleton */
/** @} Module_Base */

#endif // BASE_INC_SINGLETON_H_
