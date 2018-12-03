/**
 * @file max_heap.h
 * @brief
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_MAX_HEAP_H_
#define BASE_INC_MAX_HEAP_H_

#include <stddef.h>

/**
 * @brief 最大堆，父结点的值比左右两个子结点都大，根结点最大
 * @note 最大堆通常都是一棵完全二叉树，因此我们使用数组的形式来存储最大堆的值，从1号单元开始存储，因此父结点跟子结点的关系就是两倍的关系。\n
 *  即：heap[father * 2] = heap[leftChild];  heap[father * 2 + 1] = heap[rightChild];
 */
class MaxHeap
{
public:
    struct Data
    {
        int key;
        void* value;

        Data()
        {
            key = 0;
            value = NULL;
        }
    };

private:
    Data* array_;
    size_t size_;

    inline size_t parent(size_t i)
    {
        return i >> 1;
    }

    inline size_t left(size_t i)
    {
        return i << 1;
    }

    inline size_t right(size_t i)
    {
        return (i << 1) + 1;
    }

public:
    MaxHeap(Data* array, size_t size)
    {
        this->array_ = array;
        this->size_ = size;
    }

    void Heapify(size_t i)
    {
        size_t l, r, max = i;

        l = left(i);
        r = right(i);

        if (l < size_ && array_[i].key < array_[l].key)
        {
            max = l;
        }

        if (r < size_ && array_[max].key < array_[r].key)
        {
            max = r;
        }

        if (max != i)
        {
            Data tmp = array_[i];
            array_[i] = array_[max];
            array_[max] = tmp;

            Heapify(max);
        }
    }

    void BuildHeap()
    {
        for (int i = parent(size_ - 1); i >= 0; i--)
        {
            Heapify(i);
        }
    }

    void Sort()
    {
        Data tmp;

        for (int i = size_ - 1; i > 0; --i)
        {
            tmp = array_[0];
            array_[0] = array_[i];
            array_[i] = tmp;

            size_ = size_ - 1;
            Heapify(0);
        }
    }
};

#endif // BASE_INC_MAX_HEAP_H_
