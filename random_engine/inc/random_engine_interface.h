/**
 * @file random_engine_interface.h
 * @brief 随机数引擎
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef RANDOM_ENGINE_INC_RANDOM_ENGINE_INTERFACE_H_
#define RANDOM_ENGINE_INC_RANDOM_ENGINE_INTERFACE_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_RandomEngine 随机数引擎
 * @{
 */

#include "module_interface.h"

enum RandRange
{
    RAND_RANGE_MIN = 0,
    RAND_RANGE_ALL_CLOSED = RAND_RANGE_MIN, /**< [] */
    RAND_RANGE_FRONT_CLOSED, /**< [) */
    RAND_RANGE_BACK_CLOSED, /**< (] */
    RAND_RANGE_MAX
};

/**
 * @brief 骰子类
 * @see RandomEngineInterface::GetDice
 */
class DiceInterface
{
public:
    virtual ~DiceInterface()
    {
    }

    /**
     * @brief 以数组形式设置随机权重
     * @details 以数组形式设置随机权重，每个数组元素表示取到该元素的权重，各个元素值累加构成概率取值区间 \n
     *  如权重数组为unsigned int weight_array[] = { 10, 20, 30, 40 }; 其中有4个元素，则： \n
     *  - 取数组中第0个元素的概率为10/(10 + 20 + 30 + 40) = 10%，其概率取值区间为(0, 10]， \n
     *  - 取数组中第1个元素的概率为20/(10 + 20 + 30 + 40) = 20%，其概率取值区间为(10, 30]， \n
     *  - 取数组中第2个元素的概率为30/(10 + 20 + 30 + 40) = 30%，其概率取值区间为(30, 60]， \n
     *  - 取数组中第3个元素的概率为40/(10 + 20 + 30 + 40) = 40%，其概率取值区间为(60, 100]
     * @param weight_array 权重数组
     * @param nelems 权重数组中的元素个数
     * @attention
     *  - 并不要求数组中的各个值加起来等于100，各个权重值相当于比例
     *  - 会自动剔除权重值为0的元素，如权重数组为unsigned int weight_array[] = { 0, 10, 20, 0, 0, 30, 0, 40, 0, 0 };
     *    在掷筛子的时候只会随机到权重大于0的数组下标
     *  - 各个权重之和不能大于0xffffffff，即uint32的最大值
     * @return 返回0表示成功
     */
    virtual int SetWeights(const unsigned int weight_array[], int nelems) = 0;

    /**
     * @brief 掷一次骰子
     * @details 随机一次，返回原始的权重数组中的某个元素的下标
     * @param remove_after_rool 每次随机之后是否将本次随机到的元素剔除，不再参与下一次随机
     * @return 返回原始的权重数组中的某个元素的下标，表示随机到了该元素，范围：[0, 原始的权重数组元素个数)，返回-1表示失败
     */
    virtual int Roll(bool remove_after_rool) = 0;
};

/**
 * @brief 随机数引擎
 */
class RandomEngineInterface : public ModuleInterface
{
public:
    virtual ~RandomEngineInterface()
    {
    }

    /**
     * @brief 设置随机数种子
     */
    virtual void Seed() = 0;

    /**
     * @brief 设置指定值为随机数种子
     * @param seed 随机数种子
     */
    virtual void Seed(unsigned int seed) = 0;

    /**
     * @brief 随机生成一个布尔值
     * @return true/false
     */
    virtual bool RandBool() = 0;

    /**
     * @brief 生成一个[0, 0xffffffff]范围内的随机数，uint32
     */
    virtual unsigned int Rand() = 0;

    /**
     * @brief 生成指定区间内的一个随机数
     * @param min 区间下限
     * @param max 区间上限
     * @param range 区间开、闭标志
     * @return 生成的随机数
     * @attention 要求min < max
     */
    virtual unsigned int RandInRange(unsigned int min, unsigned int max, RandRange range) = 0;

    /**
     * @brief 生成指定区间内的无重复的n个随机数
     * @param array 存放生成的无重复的n个随机数
     * @param n 无重复的随机数个数
     * @param min 区间下限
     * @param max 区间上限
     * @param range 区间开、闭标志
     * @return 返回0表示成功
     * @attention
     *  - min < max
     *  - && n不能大于指定区间内的整数个数
     *  - && 数组长度至少为n
     */
    virtual int RandMultiNoRepeatInRange(unsigned int* array, int n, unsigned int min, unsigned int max,
                                         RandRange range) = 0;

    /**
     * @brief 生成一个随机字符串
     * @param buf
     * @param buf_size
     * @param len 随机字符串的长度
     * @param char_set
     * @param nchars
     */
    virtual void RandStr(char* buf, int buf_size, int len,
                         const char* char_set = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
                         int nchars = 62) = 0;

    /**
     * @brief 生成一个随机数，从左往右第一位不为0
     * @param buf
     * @param buf_size
     * @param len 随机数的位数
     */
    virtual void RandNumStr(char* buf, int buf_size, int len) = 0;

    /**
     * @brief 获取骰子对象
     */
    virtual DiceInterface* GetDice() = 0;
};

/** @} Module_RandomEngine */
/** @} Module_Base */

#endif // RANDOM_ENGINE_INC_RANDOM_ENGINE_INTERFACE_H_
