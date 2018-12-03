#ifndef RANDOM_ENGINE_SRC_RANDOM_ENGINE_H_
#define RANDOM_ENGINE_SRC_RANDOM_ENGINE_H_

#include "dice.h"

namespace random_engine
{
class RandomEngine : public RandomEngineInterface
{
public:
    RandomEngine();
    virtual ~RandomEngine();

    ///////////////////////// ModuleInterface /////////////////////////
    virtual const char* GetVersion() const;
    virtual const char* GetLastErrMsg() const;
    virtual void Release();
    virtual int Initialize(const void* ctx);
    virtual void Finalize();
    virtual int Activate();
    virtual void Freeze();

    ///////////////////////// RandomEngineInterface /////////////////////////
    virtual void Seed();
    virtual void Seed(unsigned int seed);
    virtual bool RandBool();
    virtual unsigned int Rand();
    virtual unsigned int RandInRange(unsigned int min, unsigned int max, RandRange range);
    virtual int RandMultiNoRepeatInRange(unsigned int* array, int n, unsigned int min, unsigned int max,
                                         RandRange range);
    void RandStr(char* buf, int buf_size, int len, const char* char_set, int nchars) override;
    void RandNumStr(char* buf, int buf_size, int len) override;
    virtual DiceInterface* GetDice();

private:
    unsigned int high_;
    unsigned int low_;
    Dice dice_;
};
}

#endif // RANDOM_ENGINE_SRC_RANDOM_ENGINE_H_
