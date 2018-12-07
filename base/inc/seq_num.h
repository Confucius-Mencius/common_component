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

#include "exception_util.h"
#include "hash_container.h"

DEFINE_EXCEPTION(SeqNumException);

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
template<typename T, T seq_min, T seq_max>
class SeqNum
{
public:
    SeqNum()
    {
        if (seq_min < 1 || seq_max <= seq_min)
        {
            THROW_EXCEPTION(SeqNumException, "invalid params! seq_min: " << seq_min << ", seq_max: " << seq_max);
        }

        Reset();
    }

    ~SeqNum()
    {
        inuse_seqs_.clear();
    }

    /**
     * @brief 清掉历史记录重新开始
     */
    void Reset()
    {
        total_count_ = seq_max - seq_min + 1;
        cursor_ = seq_min - 1;
        inuse_seqs_.clear();
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
     * @return 下一个可用的序列号，如果找不到可用的序列号则返回INVALID_SEQ，调用者要判断返回值
     * @attention 该序列号使用完毕时，需要调用Free释放它，否则该序列号就一直处于使用状态
     * @see Free INVALID_SEQ
     */
    T Alloc()
    {
        if (inuse_seqs_.size() == total_count_)
        {
            return INVALID_SEQ_NUM;
        }

        while (true)
        {
            if ((unsigned long) cursor_ < seq_max)
            {
                ++cursor_;
            }
            else
            {
                cursor_ = seq_min;
            }

            if (inuse_seqs_.insert(cursor_).second)
            {
                break;
            }
        }

        return cursor_;
    }

    /**
     * @brief 释放指定的序列号
     * @param seq 待释放的序列号，它必须是通过Alloc分配的
     * @see Alloc
     */
    void Free(T seq)
    {
        typename SeqHashSet::iterator it = inuse_seqs_.find(seq);
        if (it != inuse_seqs_.end())
        {
            inuse_seqs_.erase(it);
        }
    }

private:
    typedef __hash_set<T> SeqHashSet;

    unsigned long total_count_;
    T cursor_;
    SeqHashSet inuse_seqs_;
};

/**
 * @brief 32位有符号的自增序列号，取值范围为[1, 0x7fffffff]，不使用负数
 */
typedef SeqNum<int, (int) 1, (int) 0x7fffffff> I32SeqNum;

/**
 * @brief 32位无符号的自增序列号，取值范围为[1, 0xffffffff]
 */
typedef SeqNum<unsigned int, (unsigned int) 1, (unsigned int) 0xffffffff> U32SeqNum;

/** @} Module_SeqNum */
/** @} Module_Base */

#endif // BASE_INC_SEQ_NUM_H_
