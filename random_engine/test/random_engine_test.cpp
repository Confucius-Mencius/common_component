#include "random_engine_test.h"
#include "mem_util.h"
#include "num_util.h"

static const int LOOP_COUNT = 100;

RandomEngineTest::RandomEngineTest() : loader_()
{
    random_engine_ = NULL;
}

RandomEngineTest::~RandomEngineTest()
{
}

void RandomEngineTest::SetUp()
{
    if (loader_.Load("../librandom_engine.so") != 0)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    random_engine_ = (RandomEngineInterface*) loader_.GetModuleInterface();
    if (NULL == random_engine_)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    ASSERT_EQ(0, random_engine_->Initialize(NULL));
    ASSERT_EQ(0, random_engine_->Activate());

    random_engine_->Seed();
}

void RandomEngineTest::TearDown()
{
    SAFE_DESTROY_MODULE(random_engine_, loader_);
}

/**
 * @brief RandBool测试
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RandomEngineTest::RandBoolTest001()
{
    std::cout << "RandBool test..." << std::endl;
    int count_true = 0;
    int count_false = 0;

    for (int i = 0; i < LOOP_COUNT; ++i)
    {
        bool num = random_engine_->RandBool();
        std::cout << num << std::endl;
        if (num)
        {
            ++count_true;
        }
        else
        {
            ++count_false;
        }
    }

    std::cout << "count of true: " << count_true << ", count of false: " << count_false << std::endl;
}

/**
 * @brief RandU32测试，生成一个uint32范围内的随机数
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RandomEngineTest::RandTest001()
{
    std::cout << "RandTest001 test..." << std::endl;

    for (int i = 0; i < LOOP_COUNT; ++i)
    {
        unsigned int num = random_engine_->Rand();
        std::cout << num << std::endl;
    }
}

/**
 * @brief Rand测试，随机范围为[0, 10]
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RandomEngineTest::RandInRangeTest001()
{
    std::cout << "RandInRangeTest test[0, 10]..." << std::endl;
    RandInRangeTest(0, 10);
}

/**
 * @brief Rand测试，随机范围为[0, 100]
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RandomEngineTest::RandInRangeTest002()
{
    std::cout << "RandInRangeTest test[0, 100]..." << std::endl;
    RandInRangeTest(0, 100);
}

/**
 * @brief Rand测试，随机范围为[0, 10000]
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RandomEngineTest::RandInRangeTest003()
{
    std::cout << "RandInRangeTest test[0, 10000]..." << std::endl;
    RandInRangeTest(0, 10000);
}

/**
 * @brief Rand测试，随机范围为[0, 1000000]
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RandomEngineTest::RandInRangeTest004()
{
    std::cout << "RandInRangeTest test[0, 1000000]..." << std::endl;
    RandInRangeTest(0, 1000000);
}

/**
 * @brief RandArray测试
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RandomEngineTest::RandMultiNoRepeatInRangeTest001()
{
    std::cout << "RandMultipleNoDuplicateInRange test..." << std::endl;

    unsigned int num[10];
    unsigned int count = 5;

    ASSERT_EQ(0, random_engine_->RandMultiNoRepeatInRange(num, count, 10, 100, RAND_RANGE_ALL_CLOSED));

    for (unsigned int i = 0; i < count; ++i)
    {
        std::cout << num[i] << std::endl;
    }
}

/**
 * @brief Dice测试，随机过程中不移除元素，权重范围在10以内
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RandomEngineTest::DiceTest001()
{
    std::cout << "Dice test[0, 10], not remove after roll..." << std::endl;
    unsigned int weight_array[] = {1, 2, 3, 4};
    DiceTest(weight_array, COUNT_OF(weight_array), false);
}

/**
 * @brief Dice测试，随机过程中移除元素，权重范围在10以内
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RandomEngineTest::DiceTest002()
{
    std::cout << "Dice test[0, 10], remove after roll..." << std::endl;
    unsigned int weight_array[] = {1, 2, 3, 4};
    DiceTest(weight_array, COUNT_OF(weight_array), true);
}

/**
 * @brief Dice测试，随机过程中不移除元素，权重范围在100以内
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RandomEngineTest::DiceTest003()
{
    std::cout << "Dice test[0, 100], not remove after roll..." << std::endl;
    unsigned int weight_array[] = {10, 20, 30, 40};
    DiceTest(weight_array, COUNT_OF(weight_array), false);
}

/**
 * @brief Dice测试，随机过程中移除元素，权重范围在100以内
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RandomEngineTest::DiceTest004()
{
    std::cout << "Dice test[0, 100], remove after roll..." << std::endl;
    unsigned int weight_array[] = {10, 20, 30, 40};
    DiceTest(weight_array, COUNT_OF(weight_array), true);
}

/**
 * @brief Dice测试，随机过程中不移除元素，权重范围在10000以内
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RandomEngineTest::DiceTest005()
{
    std::cout << "dicer test[0, 10000], not remove after roll..." << std::endl;
    unsigned int weight_array[] = {1000, 2000, 3000, 4000};
    DiceTest(weight_array, COUNT_OF(weight_array), false);
}

/**
 * @brief Dice测试，随机过程中移除元素，权重范围在10000以内
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RandomEngineTest::DiceTest006()
{
    std::cout << "dicer test[0, 10000], remove after roll..." << std::endl;
    unsigned int weight_array[] = {1000, 2000, 3000, 4000};
    DiceTest(weight_array, COUNT_OF(weight_array), true);
}

/**
 * @brief Dice测试，随机过程中不移除元素，权重范围在1000000以内
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RandomEngineTest::DiceTest007()
{
    std::cout << "dicer test[0, 1000000], not remove after roll..." << std::endl;
    unsigned int weight_array[] = {100000, 200000, 300000, 400000};
    DiceTest(weight_array, COUNT_OF(weight_array), false);
}

/**
 * @brief Dice测试，随机过程中移除元素，权重范围在1000000以内
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RandomEngineTest::DiceTest008()
{
    std::cout << "dicer test[0, 1000000], remove after roll..." << std::endl;
    unsigned int weight_array[] = {100000, 200000, 300000, 400000};
    DiceTest(weight_array, COUNT_OF(weight_array), true);
}

/**
 * @brief 将调用函数random产生的随机数序列与调用Rand生成的随机数序列对比一下
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RandomEngineTest::RandomTest001()
{
    std::cout << "random test..." << std::endl;
    srandom((unsigned int) time(NULL));

    for (int i = 0; i < LOOP_COUNT; ++i)
    {
        long n = random();
        std::cout << n << std::endl;
    }
}

void RandomEngineTest::RandInRangeTest(unsigned int min, unsigned int max)
{
    unsigned int num;

    for (int i = 0; i < LOOP_COUNT; ++i)
    {
        num = random_engine_->RandInRange(min, max, RAND_RANGE_ALL_CLOSED);
        std::cout << num << std::endl;
        EXPECT_TRUE((num >= min) && (num <= max));
    }

    for (int i = 0; i < LOOP_COUNT; ++i)
    {
        num = random_engine_->RandInRange(min, max, RAND_RANGE_FRONT_CLOSED);
        std::cout << num << std::endl;
        EXPECT_TRUE((num >= min) && (num < max));
    }

    for (int i = 0; i < LOOP_COUNT; ++i)
    {
        num = random_engine_->RandInRange(min, max, RAND_RANGE_BACK_CLOSED);
        std::cout << num << std::endl;
        EXPECT_TRUE((num > min) && (num <= max));
    }
}

void RandomEngineTest::DiceTest(const unsigned int* const weight_array,
                                int elem_count, bool remove_after_roll)
{
    DiceInterface* dice = random_engine_->GetDice();
    ASSERT_TRUE(dice != NULL);
    dice->SetWeights(weight_array, elem_count);

    int last_idx = -1;

    for (int i = 0; i < elem_count; ++i)
    {
        int idx = dice->Roll(remove_after_roll);
        std::cout << idx << std::endl;
        EXPECT_TRUE((idx >= 0) && (idx < elem_count));

        if (remove_after_roll)
        {
            EXPECT_TRUE(idx != last_idx);
            last_idx = idx;
        }
    }
}

void RandomEngineTest::RandStrTest()
{
    char buf[64] = "";

    for (int i = 0; i < 10; ++i)
    {
        random_engine_->RandStr(buf, sizeof(buf), 8);
        std::cout << buf << std::endl;
    }
}

void RandomEngineTest::RandNumStrTest()
{
    char buf[64] = "";

    for (int i = 0; i < 10; ++i)
    {
        random_engine_->RandNumStr(buf, sizeof(buf), 6);
        std::cout << buf << std::endl;
    }
}

ADD_TEST_F(RandomEngineTest, RandBoolTest001);
ADD_TEST_F(RandomEngineTest, RandTest001);
ADD_TEST_F(RandomEngineTest, RandInRangeTest001);
ADD_TEST_F(RandomEngineTest, RandInRangeTest002);
ADD_TEST_F(RandomEngineTest, RandInRangeTest003);
ADD_TEST_F(RandomEngineTest, RandInRangeTest004);
ADD_TEST_F(RandomEngineTest, RandMultiNoRepeatInRangeTest001);
ADD_TEST_F(RandomEngineTest, DiceTest001);
ADD_TEST_F(RandomEngineTest, DiceTest002);
ADD_TEST_F(RandomEngineTest, DiceTest003);
ADD_TEST_F(RandomEngineTest, DiceTest004);
ADD_TEST_F(RandomEngineTest, DiceTest005);
ADD_TEST_F(RandomEngineTest, DiceTest006);
ADD_TEST_F(RandomEngineTest, DiceTest007);
ADD_TEST_F(RandomEngineTest, DiceTest008);
ADD_TEST_F(RandomEngineTest, RandomTest001);
ADD_TEST_F(RandomEngineTest, RandStrTest);
ADD_TEST_F(RandomEngineTest, RandNumStrTest);
