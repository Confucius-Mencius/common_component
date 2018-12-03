#ifndef RANDOM_ENGINE_SRC_DICE_H_
#define RANDOM_ENGINE_SRC_DICE_H_

#include <list>
#include "random_engine_interface.h"

namespace random_engine
{
class RandomEngine;

class Dice : public DiceInterface
{
public:
    Dice();
    virtual ~Dice();

    void SetRandomEngine(RandomEngine* random_engine);
    RandomEngine* GetRandomEngine() const;

    ///////////////////////// DiceInterface /////////////////////////
    virtual int SetWeights(const unsigned int weight_array[], int nelems);
    virtual int Roll(bool remove_after_roll);

private:
    int RefreshWeightRange();

private:
    struct WeightEntry
    {
        unsigned int orig_val; // 原始数据
        int orig_idx; // 原始数据在数组中的索引
        unsigned int min_val; // 本数据的概率区间下限
        unsigned int max_val; // 本数据的概率区间上限

        WeightEntry()
        {
            orig_val = 0;
            orig_idx = 0;
            min_val = 0;
            max_val = 0;
        }
    };

    typedef std::list<WeightEntry> WeightEntryList;

    RandomEngine* random_engine_;
    WeightEntryList weight_entry_list_;
};
}

#endif // RANDOM_ENGINE_SRC_DICE_H_
