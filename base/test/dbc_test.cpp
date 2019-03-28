#include "dbc.h"
#include "test_util.h"

namespace dbc_test
{
// 矩形
class Rect
{
public:
    Rect(int width, int height)
    {
        width_ = width;
        height_ = height;
    }

    /**
     * @brief 设置矩形的宽。
     * @param [in] width 宽。
     * @note
     *  - pre condition
     *   - 参数width必须大于0。
     *  - post condition
     *   - _width等于参数width。
     *   - _height保持原来的值不变。
     */
    virtual void SetWidth(int width)
    {
        if (width <= 0)
        {
            return;
        }

        width_ = width;
    }

    int GetWidth() const
    {
        return width_;
    }

    /**
     * @brief 设置矩形的高。
     * @param [in] height 高。
     * @note
     *  - pre condition
     *   - 参数height必须大于0。
     *  - post condition
     *   - _height等于参数height。
     *   - _width保持原来的值不变。
     */
    virtual void SetHeight(int height)
    {
        if (height <= 0)
        {
            return;
        }

        height_ = height;
    }

    int GetHeight() const
    {
        return height_;
    }

protected:
    int width_;
    int height_;
};

// 正方形
class Square : public Rect
{
public:
    Square(int edgeLen) : Rect(edgeLen, edgeLen)
    {
    }

    // 这个重载接口违背了父类接口中定的契约
    virtual void SetWidth(int width)
    {
        if (width <= 0)
        {
            return;
        }

        width_ = width;
        height_ = width_;
    }

    // 这个重载接口违背了父类接口中定的契约
    virtual void SetHeight(int height)
    {
        if (height <= 0)
        {
            return;
        }

        height_ = height;
        width_ = height_;
    }
};

void SomeFunc(Rect* rect)
{
    if (nullptr == rect)
    {
        return;
    }

//    try
//    {
    const int width = 10;
    const int oldHeight = rect->GetHeight();

    DBC_REQUIRE(width > 0, "参数width必须大于0");

    rect->SetWidth(width);

    DBC_ENSURE(rect->GetWidth() == width, "width未被正确赋值");
    DBC_ENSURE(rect->GetHeight() == oldHeight, "height应该保持原来的值不变");
//    }
//    catch (const DBCException& e) // 注意这里不能捕捉异常，否则gtest的EXPECT_THROW宏就捕捉不到这个异常了。
//    {
//        std::cout << e.what();
//    }
}

/**
 * @brief DBC测试-不符合DBC的设计将抛出异常。
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void Test001()
{
    Rect rect(1, 2);
    Square square(3);

    EXPECT_NO_THROW(SomeFunc(&rect));
    EXPECT_THROW(SomeFunc(&square), DBCException);
}

ADD_TEST(DBCTest, Test001);
}
/* namespace dbc_test */
