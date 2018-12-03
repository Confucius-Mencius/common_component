/**
 * @file simple_array.h
 * @brief 数组类模板
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_SIMPLE_ARRAY_H_
#define BASE_INC_SIMPLE_ARRAY_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_SimpleArray 数组类模板
 * @{
 */

/**
 * @brief 数组类模板
 * @param T 数组元素的类型
 * @param capacity 数组可以容纳的元素个数
 */
template<typename T, int capacity>
class SimpleArray
{
public:
    SimpleArray()
    {
        size_ = 0;
    }

    ~SimpleArray()
    {
    }

    /**
     * @brief 清理数组，将数组中的元素数量置为0
     */
    void Clear()
    {
        size_ = 0;
    }

    /**
     * @brief 检查数组是否为空
     * @return 返回true表示数组为空
     */
    bool IsEmpty() const
    {
        return (0 == size_);
    }

    /**
     * @brief 检查数组是否满了
     * @return 返回true表示数组满了，不能再添加新的元素了
     */
    bool IsFull() const
    {
        return (size_ >= capacity);
    }

    /**
     * @brief 向数组中添加一个元素
     * @param [in] d 要添加的元素
     * @return 新添加的元素的指针
     * @see IsFull
     */
    const T* AppendElem(const T& d)
    {
        if (IsFull())
        {
            return NULL;
        }

        a_[size_++] = d;
        return &(a_[size_ - 1]);
    }

    /**
     * @brief 获取数组中的元素数量
     * @return 数组中的元素数量
     */
    int Size() const
    {
        return size_;
    }

    /**
     * @brief 根据索引获取数组中的元素
     * @param [in] idx 索引，合法的取值范围为：[0, Size())
     * @return 指定索引处的数组元素的指针
     */
    const T* GetElem(int idx) const
    {
        if ((idx < 0) || (idx >= size_))
        {
            return NULL;
        }

        return &(a_[idx]);
    }

private:
    int size_;
    T a_[capacity];
};

/** @} Module_SimpleArray */
/** @} Module_Base */

#endif // BASE_INC_SIMPLE_ARRAY_H_
