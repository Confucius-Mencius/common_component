/**
 * @file mem_util.h
 * @brief
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_MEM_UTIL_H_
#define BASE_INC_MEM_UTIL_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_MemUtil placement new/delete模板
 * @{
 */

#include <string>

/**
 * @brief placement new
 * @details 从指定内存区中分配一个对象，要求其有一个无参数的构造函数
 * @param [in] buf 指定的一块内存区
 * @return 从指定内存区中分配的一个对象
 * @par 示例：
 * @code
 *  byte buf[256];
 *  CTest* p = Construct((CTest*) buf); // 分配对象
 *  ....
 *  Destory(p); // 释放对象
 * @endcode
 * @see Destory
 */
template<typename Type>
Type* Construct(Type* buf)
{
    return new(buf) Type();
}

/**
 * @brief placement new
 * @details 从指定内存区中分配一个对象，此函数接收一个现有的对象，通过拷贝构造的方式从指定内存区中分配一个新的对象
 * @param [in] buf 指定的一块内存区
 * @param [in] obj 现有的一个对象
 * @return 从指定内存区中分配的一个新对象
 * @par 示例：
 * @code
 *  CTest t;
 *  byte buf[256];
 *  CTest* p = Construct((CTest*) buf, t); // 分配对象
 *  ....
 *  Destory(p); // 释放对象
 * @endcode
 */
template<typename Type>
Type* Construct(Type* buf, const Type& obj)
{
    return ::new(buf) Type(obj);
}

/**
 * @brief placement delete
 * @details 将通过Construct分配得到的对象归还到内存区中去
 * @param [in] obj 待释放的对象
 * @attention obj 必须是通过Construct分配的
 * @see Construct
 */
template<typename Type>
void Destory(Type* obj)
{
    obj->~Type();
}

#define CREATE_FUNC(Type)\
public:\
    static Type* Create()\
    {\
        return new Type();\
    }

/**
 * @brief 释放通过new分配的单个对象
 * @param [in,out] obj 通过new分配的单个对象指针
 * @attention obj必须是通过new分配的单个对象
 * @hideinitializer
 */
#define SAFE_DELETE(obj)\
do {\
    if (obj != NULL)\
    {\
        ::delete obj;\
        obj = NULL;\
    }\
} while (0)

/**
 * @brief 释放通过new分配的对象数组
 * @param [in,out] obj_array 通过new分配的对象数组指针
 * @attention obj_array必须是通过new分配的对象数组
 * @hideinitializer
 */
#define SAFE_DELETE_ARRAY(obj_array)\
do {\
    if (obj_array != NULL)\
    {\
        ::delete [] obj_array;\
        obj_array = NULL;\
    }\
} while (0)

/**
 * @brief 安全释放具有Release方法的对象并将对象指针置为NULL
 * @param [in,out] obj 具有Release方法的对象指针
 * @hideinitializer
 */
#define SAFE_RELEASE(obj)\
do {\
    if (obj != NULL)\
    {\
        obj->Release();\
        obj = NULL;\
    }\
} while (0)

#define SAFE_RELEASE_MODULE(module, module_loader)\
do {\
    if (module != NULL)\
    {\
        module->Release();\
        module = NULL;\
    }\
    module_loader.Unload();\
} while (0)

#define SAFE_INITIALIZE_FAILED(obj, ctx) (obj != NULL && obj->Initialize(ctx) != 0)

#define SAFE_FINALIZE(obj)\
do {\
    if (obj != NULL)\
    {\
        obj->Finalize();\
    }\
} while (0)

#define SAFE_ACTIVATE_FAILED(obj) (obj != NULL && obj->Activate() != 0)

#define SAFE_FREEZE(obj)\
do {\
    if (obj != NULL)\
    {\
        obj->Freeze();\
    }\
} while (0)

#define SAFE_DESTROY(obj)\
do {\
    if (obj != NULL)\
    {\
        obj->Freeze();\
        obj->Finalize();\
        obj->Release();\
        obj = NULL;\
    }\
} while (0)

#define SAFE_DESTROY_MODULE(module, module_loader)\
do {\
    if (module != NULL)\
    {\
        module->Freeze();\
        module->Finalize();\
        module->Release();\
        module = NULL;\
    }\
    module_loader.Unload();\
} while (0)

/** @} Module_MemUtil */
/** @} Module_Base */

#endif // BASE_INC_MEM_UTIL_H_
