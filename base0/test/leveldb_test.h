#ifndef BASE_TEST_LEVELDB_TEST_H_
#define BASE_TEST_LEVELDB_TEST_H_

#include "test_util.h"

class LevelDbTest : public GTest
{
public:
    LevelDbTest();
    virtual ~LevelDbTest();

    void Test001();

private:

};

#endif // BASE_TEST_LEVELDB_TEST_H_
