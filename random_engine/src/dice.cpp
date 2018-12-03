#include "dice.h"
#include <stddef.h>
#include "random_engine.h"
#include "std_int.h"

namespace random_engine
{
Dice::Dice() : weight_entry_list_()
{
    random_engine_ = NULL;
}

Dice::~Dice()
{
}

void Dice::SetRandomEngine(RandomEngine* random_engine)
{
    random_engine_ = random_engine;
}

RandomEngine* Dice::GetRandomEngine() const
{
    return random_engine_;
}

int Dice::SetWeights(const unsigned int weight_array[], int nelems)
{
    if (nelems <= 0)
    {
        return -1;
    }

    WeightEntry entry;
    weight_entry_list_.clear();

    for (int i = 0; i < nelems; ++i)
    {
        // 剔除权重值为0的元素
        if (weight_array[i] > 0)
        {
            entry.orig_val = weight_array[i];
            entry.orig_idx = i;
            weight_entry_list_.push_back(entry);
        }
    }

    return RefreshWeightRange();
}

int Dice::Roll(bool remove_after_roll)
{
    if (NULL == random_engine_)
    {
        return -1;
    }

    if (0 == weight_entry_list_.size())
    {
        return -1;
    }

    const unsigned int num = random_engine_->RandInRange(weight_entry_list_.front().min_val,
                             weight_entry_list_.back().max_val, RAND_RANGE_BACK_CLOSED);
    int idx;

    for (WeightEntryList::iterator it = weight_entry_list_.begin(); it != weight_entry_list_.end(); ++it)
    {
        if ((num > it->min_val) && (num <= it->max_val))
        {
            idx = it->orig_idx;

            if (remove_after_roll)
            {
                weight_entry_list_.erase(it);

                if (RefreshWeightRange() != 0)
                {
                    return -1;
                }
            }

            return idx;
        }
    }

    return -1;
}

int Dice::RefreshWeightRange()
{
    WeightEntryList::iterator it;
    WeightEntryList::iterator prev;
    uint64_t weight_sum = 0;

    for (it = weight_entry_list_.begin(); it != weight_entry_list_.end(); ++it)
    {
        weight_sum += it->orig_val;
        if (weight_sum > 0xffffffff)
        {
            return -1;
        }

        it->min_val = (it == weight_entry_list_.begin()) ? 0 : prev->max_val;
        it->max_val = (unsigned int) weight_sum;
        prev = it;
    }

    return 0;
}
}
