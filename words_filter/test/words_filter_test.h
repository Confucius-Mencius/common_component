#ifndef WORDS_FILTER_TEST_WORDS_FILTER_TEST_H_
#define WORDS_FILTER_TEST_WORDS_FILTER_TEST_H_

#include "module_loader.h"
#include "test_util.h"
#include "words_filter_interface.h"

class WordsFilterTest : public GTest
{
public:
    WordsFilterTest();
    virtual ~WordsFilterTest();

    virtual void SetUp();
    virtual void TearDown();

    void LoadKeywordsFileTest001();
    void LoadKeywordsFileTest002();
    void CheckTest001();
    void CheckTest002();
    void CheckTest003();
    void FilterTest001();
    void FilterTest002();

private:
    ModuleLoader loader_;
    WordsFilterInterface* words_filter_;
};

#endif // WORDS_FILTER_TEST_WORDS_FILTER_TEST_H_
