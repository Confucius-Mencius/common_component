/**
 * @file variant.h
 * @brief 可变类型，支持i32、i64、f64、C语言字符串
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_VARIANT_H_
#define BASE_INC_VARIANT_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_Variant 可变类型，支持i32、i64、f64、C语言字符串
 * @{
 */

//C++中， 只有一个参数的构造函数(或者除了第一个参数外其余参数都有默认值的多参构造函数)， 承担了两个角色。 1 是个构造器 ，2 是个默认且隐含的类型转换操作符。
//所以， 有时候在我们写下如 AAA a = xxx， 这样的代码， 且恰好xxx的类型正好是AAA单参数构造器的参数类型， 这时候编译器就自动调用这个构造器， 创建一个AAA的对象， 相当于AAA tmp(xxx); AAA a = tmp;两句。
//在这样的构造函数前面加上explicit修饰， 指定这个构造函数只能被明确的调用/使用， 不能作为类型转换操作符被隐含的使用， 即类似AAA a = xxx只有的语句编译不过。

#include <string.h>
#include <ostream>
#include "data_types.h"
#include "type2type.h"

class Variant
{
public:
    enum Type
    {
        TYPE_MIN = 0,
        TYPE_I32 = TYPE_MIN,
        TYPE_I64,
        TYPE_F32,
        TYPE_F64,
        TYPE_STR, // str
        TYPE_BIN, // binary
        TYPE_MAX
    };

    struct Chunk
    {
        char* data;
        int len;
    };

    Variant();
    ~Variant();

    Variant(const Variant& v);

    explicit Variant(i32 val);
    explicit Variant(i64 val);
    explicit Variant(f32 val);
    explicit Variant(f64 val);
    explicit Variant(Type type, const char* val, int len);

    Variant& operator=(const Variant& v);
    Variant& operator=(i32 val);
    Variant& operator=(i64 val);
    Variant& operator=(f32 val);
    Variant& operator=(f64 val);
    Variant& operator=(std::pair<Variant::Type, Variant::Chunk> p);

    Type GetType() const;
    void SetType(Type type);

    bool TypeMatch(Type type) const;

    i32 GetValue(Type2Type<i32>) const;
    i64 GetValue(Type2Type<i64>) const;
    f32 GetValue(Type2Type<f32>) const;
    f64 GetValue(Type2Type<f64>) const;
    const Variant::Chunk& GetValue(Type2Type<const char*>) const;

    void ToString(char* buf, int buf_size) const;

    friend std::ostream& operator<<(std::ostream& out, const Variant& instance);

private:
    void SetValue(i32 val);
    void SetValue(i64 val);
    void SetValue(f32 val);
    void SetValue(f64 val);
    void SetValue(Type type, const char* val, int len);
    void Release();

private:
    Type type_;

    union U
    {
        i32 i;
        i64 l;
        f32 f;
        f64 d;
        Chunk chunk;

        U() : chunk()
        {
            memset(this, 0, sizeof(U));
        }
    };

    U u_;
};

/** @} Module_Variant */
/** @} Module_Base */

#endif // BASE_INC_VARIANT_DATA_H_
