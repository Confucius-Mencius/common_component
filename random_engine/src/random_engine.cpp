#include "random_engine.h"
#include <time.h>
#include <vector>
#include "version.h"

namespace random_engine
{
RandomEngine::RandomEngine() : dice_()
{
    high_ = 1;
    low_ = 1 ^ 0x49616E42;
}

RandomEngine::~RandomEngine()
{
}

const char* RandomEngine::GetVersion() const
{
    return RANDOM_ENGINE_RANDOM_ENGINE_VERSION;
}

const char* RandomEngine::GetLastErrMsg() const
{
    return NULL;
}

void RandomEngine::Release()
{
    delete this;
}

int RandomEngine::Initialize(const void* ctx)
{
    dice_.SetRandomEngine(this);
    return 0;
}

void RandomEngine::Finalize()
{
}

int RandomEngine::Activate()
{
    return 0;
}

void RandomEngine::Freeze()
{
}

void RandomEngine::Seed()
{
    Seed((unsigned int) time(NULL));
}

void RandomEngine::Seed(unsigned int seed)
{
    high_ = seed;
    low_ = seed ^ 0x49616E42;
}

bool RandomEngine::RandBool()
{
    return (0 == (Rand() & 1));
}

unsigned int RandomEngine::Rand()
{
    const size_t shift = 2;

    high_ = (high_ >> shift) + (high_ << shift);
    high_ += low_;
    low_ += high_;

    return high_;
}

unsigned int RandomEngine::RandInRange(unsigned int min, unsigned int max, RandRange range)
{
    const unsigned int default_ret = 0xffffffff; // default return value

    if (min >= max)
    {
        return default_ret;
    }

    if (range < RAND_RANGE_MIN || range >= RAND_RANGE_MAX)
    {
        return default_ret;
    }

    const double double_max = 0xffffffff;
    unsigned int num = default_ret;

    switch (range)
    {
        case RAND_RANGE_ALL_CLOSED:
        {
            num = (unsigned int) ((Rand() / double_max) * (max - min) + min);
        }
        break;

        case RAND_RANGE_FRONT_CLOSED:
        {
            num = (unsigned int) ((Rand() / (double_max + 1)) * (max - min) + min);
        }
        break;

        case RAND_RANGE_BACK_CLOSED:
        {
            num = (Rand() % (max - min)) + min + 1;
        }
        break;

        default:
        {
        }
        break;
    }

    return num;
}

int RandomEngine::RandMultiNoRepeatInRange(unsigned int* array, int n, unsigned int min, unsigned int max,
        RandRange range)
{
    if (NULL == array || 0xffffffff == min || max < 1)
    {
        return -1;
    }

    unsigned int real_min = min;
    unsigned int real_max = max;

    switch (range)
    {
        case RAND_RANGE_ALL_CLOSED:
        {
        }
        break;

        case RAND_RANGE_FRONT_CLOSED:
        {
            real_max = max - 1;
        }
        break;

        case RAND_RANGE_BACK_CLOSED:
        {
            real_min = min + 1;
        }
        break;

        default:
        {
            return -1;
        }
        break;
    }

    if (real_min >= real_max)
    {
        return -1;
    }

    const int nnums = (real_max - real_min) + 1; // 该范围内的整数个数
    if (n > nnums)
    {
        return -1;
    }

    std::vector<unsigned int> u32_vec;
    u32_vec.reserve((size_t) n);

    // 将该范围内的所有整数放入vector中，随机数就从这个vector中取
    for (int i = 0; i < nnums; ++i)
    {
        u32_vec.push_back(i + real_min);
    }

    unsigned int num_left = (unsigned int) nnums;

    for (int i = 0; i < n; ++i)
    {
        const unsigned int idx = Rand() % num_left;
        array[i] = u32_vec[idx];

        // 用vector尾部的值覆盖这个已经随机到的值
        const unsigned int tmp = u32_vec[num_left - 1];
        u32_vec[idx] = tmp;

        --num_left; // 移除这个随机到的值，保证不重复
    }

    u32_vec.clear();
    return 0;
}

void RandomEngine::RandStr(char* buf, int buf_size, int len, const char* char_set, int nchars)
{
    if (NULL == buf || len < 1 || len > (buf_size - 1) || NULL == char_set || nchars < 1)
    {
        return;
    }

    for (int i = 0; i < len; ++i)
    {
        int idx = RandInRange(0, nchars, RAND_RANGE_FRONT_CLOSED);
        buf[i] = char_set[idx];
    }
}

void RandomEngine::RandNumStr(char* buf, int buf_size, int len)
{
    if (NULL == buf || len < 1 || len > (buf_size - 1))
    {
        return;
    }

    static const char char_set[] = "0123456789";

    for (int i = 1; i < len; ++i)
    {
        int idx = RandInRange(0, 10, RAND_RANGE_FRONT_CLOSED);
        buf[i] = char_set[idx];
    }

    const char char_set_without_0[] = "123456789";
    int idx = RandInRange(0, 9, RAND_RANGE_FRONT_CLOSED);
    buf[0] = char_set_without_0[idx];
}

DiceInterface* RandomEngine::GetDice()
{
    return &dice_;
}
}
