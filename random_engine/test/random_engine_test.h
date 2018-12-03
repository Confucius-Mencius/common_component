#ifndef RANDOM_ENGINE_TEST_RANDOM_ENGINE_TEST_H_
#define RANDOM_ENGINE_TEST_RANDOM_ENGINE_TEST_H_

#include "module_loader.h"
#include "random_engine_interface.h"
#include "test_util.h"

class RandomEngineTest : public GTest
{
public:
    RandomEngineTest();
    virtual ~RandomEngineTest();

    virtual void SetUp();
    virtual void TearDown();

    void RandBoolTest001();
    void RandTest001();
    void RandInRangeTest001();
    void RandInRangeTest002();
    void RandInRangeTest003();
    void RandInRangeTest004();
    void RandMultiNoRepeatInRangeTest001();
    void DiceTest001();
    void DiceTest002();
    void DiceTest003();
    void DiceTest004();
    void DiceTest005();
    void DiceTest006();
    void DiceTest007();
    void DiceTest008();
    void RandomTest001();
    void RandStrTest();
    void RandNumStrTest();

private:
    void RandInRangeTest(unsigned int min, unsigned int max);
    void DiceTest(const unsigned int* const weight_array, int elem_count, bool remove);

private:
    ModuleLoader loader_;
    RandomEngineInterface* random_engine_;
};

#endif // RANDOM_ENGINE_TEST_RANDOM_ENGINE_TEST_H_
