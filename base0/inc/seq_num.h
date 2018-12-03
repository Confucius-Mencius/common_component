/**
 * @file seq_num.h
 * @brief 自增序列号
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_SEQ_NUM_H_
#define BASE_INC_SEQ_NUM_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_SeqNum 自增序列号
 * @{
 */

#include "hash_container.h"

/**
 * @brief 非法序列号
 */
#define INVALID_SEQ_NUM 0

/**
 * @brief 自增序列号类模板，序列号的取值范围为：[seq_min, seq_max]，分配序列号的时候每次加1递增到最大值，然后回绕到最小值，如此循环
 * @param T 序列号的类型，为int32或uint32
 * @param seq_min 序列号的最小值，必须大于0
 * @param seq_max 序列号的最大值，必须大于seq_min
 */
template<typename T, unsigned long seq_min, unsigned long seq_max>
class SeqNum
{
public:
    SeqNum()
    {
        if (seq_min < 1 || seq_max <= seq_min)
        {
            return;
        }

        Reset();
    }

    ~SeqNum()
    {
        used_seq_hash_set_.clear();
    }

    /**
     * @brief 清掉历史记录重新开始
     */
    void Reset()
    {
        seq_cursor_ = seq_min - 1;
        overflow_flag_ = 0;
        used_seq_hash_set_.clear();
    }

    /**
     * @brief 获取序列号的最小值
     * @return 序列号的最小值
     */
    T Min() const
    {
        return seq_min;
    }

    /**
     * @brief 获取序列号的最大值
     * @return 序列号的最大值
     */
    T Max() const
    {
        return seq_max;
    }

    /**
     * @brief 分配下一个可用的序列号，如果下一个序列号在使用中还未释放，则顺次加1，直到找到一个空闲的序列号为止
     * @return 下一个可用的序列号，如果找不到可用的序列号则返回INVALID_SEQ，调用者需要进行判断
     * @attention 该序列号使用完毕时，需要调用Free释放它，否则该序列号就一直处于使用状态
     * @see Free INVALID_SEQ
     */
    T Alloc()
    {
        if ((unsigned long) seq_cursor_ < seq_max)
        {
            ++seq_cursor_;
        }
        else
        {
            ++overflow_flag_;
            seq_cursor_ = seq_min;
        }

        if (overflow_flag_ > 0)
        {
            unsigned int loop = 0;

            while (true)
            {
                if (used_seq_hash_set_.find(seq_cursor_) == used_seq_hash_set_.end())
                {
                    break;
                }

                ++loop;

                if (loop >= (seq_max - seq_min + 1))
                {
                    return INVALID_SEQ_NUM;
                }

                ++seq_cursor_;
            }
        }

        if (!used_seq_hash_set_.insert(seq_cursor_).second)
        {
            return INVALID_SEQ_NUM;
        }

        return seq_cursor_;
    }

    /**
     * @brief 释放指定的序列号
     * @param seq 待释放的序列号，它必须是通过Alloc分配的
     * @see Alloc
     */
    void Free(T seq)
    {
        typename SeqHashSet::iterator it = used_seq_hash_set_.find(seq);
        if (it != used_seq_hash_set_.end())
        {
            used_seq_hash_set_.erase(it);
        }
    }

private:
    typedef __hash_set<T> SeqHashSet;

    T seq_cursor_;
    SeqHashSet used_seq_hash_set_;
    int overflow_flag_;
};

/**
 * @brief 32位有符号的自增序列号，取值范围为[1, 0x7fffffff]，不使用负数
 */
typedef SeqNum<int, 1, (int) 0x7fffffff> I32SeqNum;

/**
 * @brief 32位无符号的自增序列号，取值范围为[1, 0xffffffff]
 */
typedef SeqNum<unsigned int, 1, (unsigned int) 0xffffffff> U32SeqNum;

/** @} Module_SeqNum */
/** @} Module_Base */

#endif // BASE_INC_SEQ_NUM_H_
