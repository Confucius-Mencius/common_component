#include "variant.h"
#include "num_str_util.h"

static const char EMPTY_STR[] = "";
static const Variant::Chunk EMPTY_CHUNK = { (char*) EMPTY_STR, 0 };

Variant::Variant() : u_()
{
    type_ = TYPE_MAX;
}

Variant::~Variant()
{
    Release();
}

Variant::Variant(const Variant& v)
{
    if (&v != this)
    {
        if (TYPE_STR == v.type_ || TYPE_BIN == v.type_)
        {
            // 深拷贝
            SetValue(v.type_, v.u_.chunk.data, v.u_.chunk.len);
        }
        else
        {
            type_ = v.type_;
            u_ = v.u_;
        }
    }
}

Variant::Variant(i32 val)
{
    SetValue(val);
}

Variant::Variant(i64 val)
{
    SetValue(val);
}

Variant::Variant(f32 val)
{
    SetValue(val);
}

Variant::Variant(f64 val)
{
    SetValue(val);
}

Variant::Variant(Type type, const char* val, int len)
{
    SetValue(type, val, len);
}

Variant& Variant::operator=(const Variant& v)
{
    if (&v != this)
    {
        Release();

        if (TYPE_STR == v.type_ || TYPE_BIN == v.type_)
        {
            // 深拷贝
            SetValue(v.type_, v.u_.chunk.data, v.u_.chunk.len);
        }
        else
        {
            type_ = v.type_;
            u_ = v.u_;
        }
    }

    return *this;
}

Variant& Variant::operator=(i32 val)
{
    Release();
    SetValue(val);

    return *this;
}

Variant& Variant::operator=(i64 val)
{
    Release();
    SetValue(val);

    return *this;
}

Variant& Variant::operator=(f32 val)
{
    Release();
    SetValue(val);

    return *this;
}

Variant& Variant::operator=(f64 val)
{
    Release();
    SetValue(val);

    return *this;
}

Variant& Variant::operator=(std::pair<Variant::Type, Variant::Chunk> p)
{
    Release();
    SetValue(p.first, p.second.data, p.second.len);

    return *this;
}

Variant::Type Variant::GetType() const
{
    return type_;
}

void Variant::SetType(Type type)
{
    type_ = type;
}

bool Variant::TypeMatch(Type type) const
{
    return (type_ == type);
}

i32 Variant::GetValue(Type2Type<i32>) const
{
    if (type_ != TYPE_I32)
    {
        return 0;
    }

    return u_.i;
}

i64 Variant::GetValue(Type2Type<i64>) const
{
    if (type_ != TYPE_I64)
    {
        return 0;
    }

    return u_.l;
}

f32 Variant::GetValue(Type2Type<f32>) const
{
    if (type_ != TYPE_F32)
    {
        return 0;
    }

    return u_.f;
}

f64 Variant::GetValue(Type2Type<f64>) const
{
    if (type_ != TYPE_F64)
    {
        return 0;
    }

    return u_.d;
}

const Variant::Chunk& Variant::GetValue(Type2Type<const char*>) const
{
    if (type_ != TYPE_STR && type_ != TYPE_BIN)
    {
        return EMPTY_CHUNK;
    }

    return u_.chunk;
}

void Variant::ToString(char* buf, int buf_size) const
{
    switch (type_)
    {
        case TYPE_I32:
        {
            Num2Str(buf, buf_size, Type2Type<i32>(), u_.i);
        }
        break;

        case TYPE_I64:
        {
            Num2Str(buf, buf_size, Type2Type<i64>(), u_.l);
        }
        break;

        case TYPE_F32:
        {
            Num2Str(buf, buf_size, Type2Type<f32>(), u_.f);
        }
        break;

        case TYPE_F64:
        {
            Num2Str(buf, buf_size, Type2Type<f64>(), u_.d);
        }
        break;

        case TYPE_STR:
        {
            StrCpy(buf, buf_size, u_.chunk.data);
        }
        break;

        case TYPE_BIN:
        {
            memmove(buf, u_.chunk.data, u_.chunk.len > buf_size ? buf_size : u_.chunk.len);
        }
        break;

        default:
        {
        }
        break;
    }
}

std::ostream& operator<<(std::ostream& out, const Variant& instance)
{
    switch (instance.type_)
    {
        case Variant::TYPE_I32:
        {
            out << instance.u_.i;
        }
        break;

        case Variant::TYPE_I64:
        {
            out << instance.u_.l;
        }
        break;

        case Variant::TYPE_F32:
        {
            out << instance.u_.f;
        }
        break;

        case Variant::TYPE_F64:
        {
            out << instance.u_.d;
        }
        break;

        case Variant::TYPE_STR:
        {
            out << instance.u_.chunk.data << "(" << instance.u_.chunk.len << ")";
        }
        break;

        case Variant::TYPE_BIN:
        {
            out << instance.u_.chunk.data << "(" << instance.u_.chunk.len << ")";
        }
        break;

        default:
        {
        }
        break;
    }

    return out;
}

void Variant::SetValue(i32 val)
{
    type_ = TYPE_I32;
    u_.i = val;
}

void Variant::SetValue(i64 val)
{
    type_ = TYPE_I64;
    u_.l = val;
}

void Variant::SetValue(f32 val)
{
    type_ = TYPE_F32;
    u_.f = val;
}

void Variant::SetValue(f64 val)
{
    type_ = TYPE_F64;
    u_.d = val;
}

void Variant::SetValue(Type type, const char* val, int len)
{
    if (NULL == val || len < 0)
    {
        return;
    }

    if (type != TYPE_STR && type != TYPE_BIN)
    {
        return;
    }

    type_ = type;

    if (0 == len)
    {
        u_.chunk.data = (char*) EMPTY_STR;
        u_.chunk.len = 0;
    }
    else
    {
        u_.chunk.data = new char[len + 1];
        if (NULL == u_.chunk.data)
        {
            return;
        }

        if (TYPE_STR == type_)
        {
            StrCpy(u_.chunk.data, len + 1, val);
        }
        else
        {
            memmove(u_.chunk.data, val, len);
        }

        u_.chunk.len = len;
    }
}

void Variant::Release()
{
    if (TYPE_STR == type_ || TYPE_BIN == type_)
    {
        if (u_.chunk.data != NULL && u_.chunk.len > 0) // 因为可能为EMPTY_STR，必须判断长度>0
        {
            delete[] u_.chunk.data;
            u_.chunk.data = NULL;
            u_.chunk.len = 0;
        }
    }
}
