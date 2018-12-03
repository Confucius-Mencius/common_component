#ifndef BASE_TEST_ZIP_TEST_H_
#define BASE_TEST_ZIP_TEST_H_

#include "test_util.h"

class ZipTest : public GTest
{
public:
    ZipTest();
    virtual ~ZipTest();

    void Test001(); // 中文名
    void Test002(); // 小文件
    void Test003(); // 两个大文件+一个空文件，空文件偏移超过4G
    void Test004(); // 一个大文件（超过4G）
    void Test005(); // 两个大文件（超过4G）
    void Test006();

private:

};

#endif // BASE_TEST_ZIP_TEST_H_
