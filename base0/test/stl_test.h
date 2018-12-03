#ifndef BASE_TEST_STL_TEST_H_
#define BASE_TEST_STL_TEST_H_

#include "test_util.h"

class STLTest : public GTest
{
public:
    STLTest();
    virtual ~STLTest();

    // 验证在遍历过程中删除符合条件的元素的方法
    void Test_Vector();
    void Test_List();
    void Test_Map();
    void Test_Set();
    void Test_Multimap();
    void Test_Multiset();
    void Test_HashMap();
    void Test_HashMultimap();

    void Test_TraverseMap();

    void Test_Vector_end();

private:

};

#endif // BASE_TEST_STL_TEST_H_
