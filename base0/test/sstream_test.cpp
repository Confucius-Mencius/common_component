#include "test_util.h"

//在C++有两种字符串流,也称为数组I/O流，一种在sstream中定义，
//另一种在strstream中定义。
//它们实现的东西基本一样。
//
//strstream里包含
//class strstreambuf;
//class istrstream;
//class ostrstream;
//class strstream;
//它们是基于C类型字符串char*编写的
//
//sstream中包含
//class istringstream;
//class ostringstream;
//class stringbuf;
//class stringstream;
//class …….
//它们是基于std::string编写的
//
//因此ostrstream::str()返回的是char*类型的字符串
//而ostringstream::str()返回的是std::string类型的字符串
//
//在使用的时候要注意到二者的区别，一般情况下推荐使用std::string类型的字符串
//当然如果为了保持和C的兼容，使用strstream也是不错的选择。
//但要记住一点,strstream虽仍然是C++语言标准的一部分,但已被C++标准宣称为“deprecated”，也就是不再提倡使用了

using namespace std;

namespace sstream_test
{
/**
 * @brief 格式化输出测试
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
    // 将整数以16进制的格式（默认是小写的）输出
    std::ostringstream oss1;
    oss1 << hex << showbase << 255; // showbase指要输出小写进制前缀，这里是0x
    cout << oss1.str() << endl; // 0xff

    // 将字节序列以16进制（默认是小写的）的方式输出
    std::ostringstream oss2;
    unsigned char x = 255;
    unsigned char y = 16;
    unsigned char z = 1;
    oss2 << hex << setfill('0');
    oss2 << setw(2) << (int) x; // 一定是输出一个int，否则无效。
    oss2 << setw(2) << (int) y; // 一定是输出一个int，否则无效。
    oss2 << setw(2) << (int) z; // 一定是输出一个int，否则无效。
    cout << oss2.str() << endl; // ff1001

    // 以16进制大写的格式输出
    std::ostringstream oss3;
    oss3 << hex << showbase << uppercase << 255;
    cout << oss3.str() << endl; // 0XFF

    // 以32位整数的方式输出，在前面通常要补上多个0
    std::ostringstream oss4;
    oss4 << "0x" << hex << uppercase << setw(8) << setfill('0') << 255;
    cout << oss4.str() << endl; // 0x000000FF

    // 格式化浮点数，在小数点后保留6位：
    std::ostringstream oss5;
    oss5 << fixed << setprecision(6) << 1234.5678; // 将输出1234.567800。fixed的作用是就算全是0也一定保留6位小数。
    cout << oss5.str() << endl; // 1234.567800

    // 保留3位小数，将输出1234.568，已经自动做了四舍五入
    std::ostringstream oss6;
    oss6 << fixed << setprecision(3) << 1234.5678;
    cout << oss6.str() << endl; // 1234.568

    // 当用ostringstream格式化完毕后，通过调用它的str()成员函数可以得到格式化后的字符串：string s = ost.str();

    // 如果接下来要继续在这个流对象上进行其它的格式化工作，那么要先清空ostringstream的缓存，
    // 传递一个空字符串即可: ost.str("");，用clear是没有的，因为clear只是清除出错标记。
}

void Test002()
{
    const double value = 12.3456789;

    cout << value << endl; // 默认以6精度，所以输出为 12.3457
    cout << setprecision(4) << value << endl; // 改成4精度，所以输出为12.35
    cout << setprecision(8) << value << endl; // 改成8精度，所以输出为12.345679
    cout << fixed << setprecision(4) << value << endl; // 加了fixed意味着是固定点方式显示，所以这里的精度指的是小数位，输出为12.3457
    cout << value << endl; // fixed和setprecision的作用还在，依然显示12.3457

    cout.unsetf(ios::fixed); // 去掉了fixed，所以精度恢复成整个数值的有效位数，显示为12.35
    cout << value << endl;

    cout.precision(6); // 恢复成原来的样子，输出为12.3457
    cout << value << endl;
}

void Test003()
{
    float f1 = 0.1f;
    float f2 = 1.0f;
    int i = 6900;

    float f = i * (1 - f1 * f2);
    int x = (int) (i * (1 - f1 * f2));
    int y = (int) ceil(i * (1 - f1 * f2));
    int z = (int) floor(i * (1 - f1 * f2));

    printf("%f, %d, %d, %d\n", f, x, y, z); // 输出：6210.000000, 6209, 6210, 6210
}

ADD_TEST(SStreamTest, Test001);
ADD_TEST(SStreamTest, Test002);
ADD_TEST(SStreamTest, Test003);
}
/* namespace sstream_test */
