#ifndef WORDS_FILTER_TEST_CONV_TEST_H_
#define WORDS_FILTER_TEST_CONV_TEST_H_

#include "test_util.h"

class ConvTest : public GTest
{
public:
    ConvTest();
    virtual ~ConvTest();

    void UTF8UnicodeConvTest();
    void UTF8TradSimpConvTest();
    void UTFUnicodeConvTest();
};

#endif // WORDS_FILTER_TEST_CONV_TEST_H_
