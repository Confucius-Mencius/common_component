#include "boost_test.h"
#include "boost/bind.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/scoped_ptr.hpp"
#include "boost/locale.hpp"
#include "boost/circular_buffer.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include "str_util.h"

using namespace std;

// 普通函数
static void fun(int x, int y)
{
    cout << x << ", " << y << endl;
}

// 函数对象
static struct Func
{
    void operator()(int x)
    {
        cout << x << endl;
    }
} f;

// 非静态成员函数
struct A
{
    A()
    {
        cout << "A" << endl;
    }

    ~A()
    {
        cout << "~A" << endl;
    }

    void func(int x, int y)
    {
        cout << x << "," << y << endl;
    }
};

class B
{
public:
    B()
    {
        cout << "B" << endl;
    }

    ~B()
    {
        cout << "~B" << endl;
    }

    void DoSomething()
    {
        cout << "do something" << endl;
    }
};

static void ScopedPtrTest()
{
    boost::scoped_ptr<B> impl(new B());
    impl->DoSomething();
}

static void SharedPtrTest()
{
    boost::shared_ptr<B> sp1(new B());
    cout << "The Sample now has " << sp1.use_count() << " references" << endl;

    boost::shared_ptr<B> sp2 = sp1;
    cout << "The Sample now has " << sp2.use_count() << " references" << endl;

    sp1.reset();
    cout << "After Reset sp1. The Sample now has " << sp2.use_count() << " references" << endl;

    sp2.reset();
    cout << "After Reset sp2." << endl;
}

typedef std::vector<std::string> TokenVec;

static void SplitTest(const std::string& s)
{
    try
    {
        std::vector<std::string> split_vec_and;
        boost::split(split_vec_and, s, boost::is_any_of("&"), boost::token_compress_on);
        const int and_vec_size = (int) split_vec_and.size();

        for (int j = 0; j < and_vec_size; ++j)
        {
            std::cout << split_vec_and[j] << std::endl;
        }

        std::cout << "& -----" << std::endl;

        std::ostringstream pure_data_oss;
        TokenVec token_vec;
        int m = 0;

        for (int j = 0; j < and_vec_size; ++j)
        {
            std::cout << split_vec_and[j] << std::endl;

            std::string::size_type pos = split_vec_and[j].find("=");
            if (pos == std::string::npos || (pos + 1) == split_vec_and[j].length())
            {
                std::cout << "invalid format: " << split_vec_and[j] << std::endl;
                continue;
            }

            std::string orig_token(split_vec_and[j].substr(pos + 1));
            std::cout << "orig token: " << orig_token << std::endl;

            std::vector<std::string> split_vec_equal;
            boost::split(split_vec_equal, split_vec_and[j], boost::is_any_of("="));
            const int equal_vec_size = (int) split_vec_equal.size();

            for (int k = 0; k < equal_vec_size; ++k)
            {
                std::cout << split_vec_equal[k] << std::endl;
            }

            std::cout << "= -----" << std::endl;

            if (2 == equal_vec_size)
            {
                const int buf_size = split_vec_equal[1].length() + 1;
                char* token = new char[buf_size];
                int n = StrReplace(token, buf_size, split_vec_equal[1].c_str(), ",", ";");
                (void) n;

                if (0 == m)
                {
                    pure_data_oss << token;
                }
                else
                {
                    pure_data_oss << "," << token;
                }

                token_vec.push_back(token);
                ++m;
                delete[] token;
            }
            else
            {
                std::cout << "not 2 item!!!!!!!!!!" << std::endl;
            }
        }

        std::cout << "-----ok. " << pure_data_oss.str() << std::endl;
    }
    catch (...)
    {
        std::cout << "failed to split str: " << s << std::endl;
        return;
    }
}

BoostTest::BoostTest()
{

}

BoostTest::~BoostTest()
{

}

void BoostTest::Test_Bind()
{
    boost::bind(&fun, 3, 4)();  //无参数
    boost::bind(&fun, 3, _1)(4);  //这个4 会代替 _1 占位符.
    boost::bind(&fun, _1, _2)(3, 4);    // 3将代替_1占位符, 4将代替_2占位符.
    boost::bind(&fun, _2, _1)(3, 4);   // 3将代替_1占位符, 4将代替_2占位符.

    boost::bind<void>(f, 3)();  //指出返回值的类型 void

    A a;
    boost::bind(&A::func, a, 3, 4)();    //输出 3, 4

    A* pa = new A; //指针
    boost::shared_ptr<A> ptr_a(pa);  //智能指针.

    boost::bind(&A::func, pa, 3, 4)();   //输出 3, 4
    boost::bind(&A::func, ptr_a, 3, 4)();//输出 3, 4
    boost::bind(&A::func, _1, 3, 4)(ptr_a);//输出 3, 4
}

void BoostTest::Test_Ptr()
{
    // scoped_ptr，不能用在stl容器中，不能用于数组
    ScopedPtrTest();

    // shared_ptr，不能处理循环引用
    SharedPtrTest();
}

void BoostTest::Test_Locale()
{
//    Boost.Locale提供强大的工具，跨平台的本地化软件开发。
//    提供的功能：
//    正确的大小写转换，大小写和规范化。
//    整理（排序），其中包括4 Unicode排序规则层面的支持。
//    日期，时间，时区和日历操作，格式化和分析，包括对日历比公历其他透明的支持。
//    边界分析字，词，句和换行符。
//    数字格式，拼写和语法分析。
//    货币格式和解析。
//    强大的邮件格式（字符串翻译），包括多种格式，使用GNU目录的支持。
//    字符集转换。
//    8位字符透明支持集，如拉丁文
//            支持的char和wchar_t的
//    对C++0x的实验性支持char16_t和char32_t字符串和数据流。

    string utf8_str = "我爱你";
    wstring utf8_wstr = L"我爱你";

    std::string gbk_str = boost::locale::conv::between(utf8_str, "GBK", "UTF-8");

    // from_utf 是把 UTF 字串（string 或wstring）、转换为特定编码的字串用的，他可以转换string 或wstring 的字串，但是输出一定是string
    string gbk_str1 = boost::locale::conv::from_utf(utf8_str, "GBK");
    string gbk_str2 = boost::locale::conv::from_utf(utf8_wstr, "GBK");

    EXPECT_STREQ(gbk_str.c_str(), gbk_str1.c_str());
    EXPECT_STREQ(gbk_str.c_str(), gbk_str2.c_str());

    // to_utf可以输出 string ，也可以输出成 wstring
    string utf8_str1 = boost::locale::conv::to_utf<char>(gbk_str2, "GBK");
    wstring utf8_wstr1 = boost::locale::conv::to_utf<wchar_t>(gbk_str1, "GBK");

    EXPECT_STREQ(utf8_str.c_str(), utf8_str1.c_str());
    EXPECT_STREQ(utf8_wstr.c_str(), utf8_wstr1.c_str());

    // TODO 用BIG5编码时，from_utf和to_utf无法可逆！！

    // utf_to_utf的目的，是在 UTF 的 string 字串和 wstring 字串之间做转换
    string utf8_str2 = boost::locale::conv::utf_to_utf<char>(utf8_wstr);
    wstring utf8_wstr2 = boost::locale::conv::utf_to_utf<wchar_t>(utf8_str);

    EXPECT_STREQ(utf8_str.c_str(), utf8_str2.c_str());
    EXPECT_STREQ(utf8_wstr.c_str(), utf8_wstr2.c_str());
}

void BoostTest::Test_CircularBuffer_int()
{
    // Create a circular buffer with a capacity for 3 integers.
    boost::circular_buffer<int> cb;
    cb.resize(3);

    // Insert some elements into the buffer.
    cb.push_back(1);
    std::cout << cb.size() << ", " << cb.max_size() << ", " << cb.capacity() << ", " << cb.is_linearized() << std::endl;

    cb.push_back(2);
    std::cout << cb.size() << ", " << cb.max_size() << ", " << cb.capacity() << ", " << cb.is_linearized() << std::endl;

    cb.push_back(3);
    std::cout << cb.size() << ", " << cb.max_size() << ", " << cb.capacity() << ", " << cb.is_linearized() << std::endl;

    int a = cb[0]; // a == 1
    std::cout << a << std::endl;

    int b = cb[1]; // b == 2
    std::cout << b << std::endl;

    int c = cb[2]; // c == 3
    std::cout << c << std::endl;

    // The buffer is full now, pushing subsequent
    // elements will overwrite the front-most elements.

    cb.push_back(4); // Overwrite 1 with 4.
    std::cout << cb.size() << ", " << cb.max_size() << ", " << cb.capacity() << ", " << cb.is_linearized() << std::endl;

    cb.push_back(5); // Overwrite 2 with 5.
    std::cout << cb.size() << ", " << cb.max_size() << ", " << cb.capacity() << ", " << cb.is_linearized() << std::endl;

    // The buffer now contains 3, 4 and 5.
    a = cb[0]; // a == 3
    b = cb[1]; // b == 4
    c = cb[2]; // c == 5

    // Elements can be popped from either the front or the back.
    cb.pop_back(); // 5 is removed.
    std::cout << cb.size() << ", " << cb.max_size() << ", " << cb.capacity() << ", " << cb.is_linearized() << std::endl;

    cb.pop_front(); // 3 is removed.
    std::cout << cb.size() << ", " << cb.max_size() << ", " << cb.capacity() << ", " << cb.is_linearized() << std::endl;

    int d = cb[0]; // d == 4
    std::cout << d << std::endl;
}

void BoostTest::Test_CircularBuffer_char()
{
    boost::circular_buffer<char> cb(5);
    cb.push_back('a');
    cb.push_back('b');

    char ch[20] = {0};
    strcpy(ch, "helloworld");
    int len = strlen(ch);
    cout << len << endl;

    cb.insert(cb.end(), &ch[0], &ch[len]); // 使用尾插法
    cout << "begin: " << *(cb.begin() + 6 % cb.size()) << endl; // 使用迭代器不能超出circular_buffer的大小
    cout << "end: " << *(cb.end() - 1) << endl; // 最后一个元素的位置为end()-1

    std::string str2;
    str2.append(cb.begin(), cb.end()); // 将circular_buffer中的数据全部保存到string中
    cout << str2 << endl;
}

void BoostTest::Test_CircularBuffer_ReloadSize()
{
    boost::circular_buffer<char> buf;
    buf.set_capacity(1024);
    std::cout << buf.capacity() << ", " << buf.size() << std::endl;

    char ch[20] = {0};
    strcpy(ch, "helloworld");
    int len = strlen(ch);
    cout << len << endl;

    buf.insert(buf.end(), &ch[0], &ch[len]);
    std::cout << buf.capacity() << ", " << buf.size() << std::endl;

    buf.set_capacity(512);
    std::cout << buf.capacity() << ", " << buf.size() << std::endl;

}

void BoostTest::Test_Split()
{
    // 正常
    std::string s1 = "a=1&b=2";
    std::cout << s1 << std::endl;
    SplitTest(s1);

    // value为空  x
    std::string s2 = "a=1&b=";
    std::cout << s2 << std::endl;
    SplitTest(s2);

    // key为空
    std::string s3 = "a=1&=2";
    std::cout << s3 << std::endl;
    SplitTest(s3);

    // value中含有=号
    std::string s4 = "a=1&b=2=3";
    std::cout << s4 << std::endl;
    SplitTest(s4);

    // 两个&&之间无内容
    std::string s5 = "a=1&&b=2";
    std::cout << s5 << std::endl;
    SplitTest(s5);

    // 无=号  x
    std::string s6 = "a=1&&b";
    std::cout << s6 << std::endl;
    SplitTest(s6);

    // 只有一个=号，无key/value  x
    std::string s7 = "a=1&&=";
    std::cout << s7 << std::endl;
    SplitTest(s7);
}

void BoostTest::Test_Path1()
{
    boost::filesystem::path p1("/folder1/folder2/folder3/filename.ext");
//    std::cout << EXPAND_MACRO(PRINT_OF(p1, string())) << std::endl;

    PRINT_OF(p1, string());
    PRINT_OF(p1, root_name());
    PRINT_OF(p1, root_directory());
    PRINT_OF(p1, root_path());
    PRINT_OF(p1, relative_path());
    PRINT_OF(p1, filename());
    PRINT_OF(p1, parent_path());
    PRINT_OF(p1, stem());
    PRINT_OF(p1, extension());

    PRINT_OF(p1, replace_extension("new"));
    char buf[] = "hello";
    PRINT_OF(p1, append(buf, buf + sizeof(buf)));
    PRINT_OF(p1, remove_filename());
}

void BoostTest::Test_Path2()
{
    boost::filesystem::path p1("folder1/folder2/folder3/filename.ext");
//    std::cout << EXPAND_MACRO(PRINT_OF(p1, string())) << std::endl;

    PRINT_OF(p1, string());
    PRINT_OF(p1, root_name());
    PRINT_OF(p1, root_directory());
    PRINT_OF(p1, root_path());
    PRINT_OF(p1, relative_path());
    PRINT_OF(p1, filename());
    PRINT_OF(p1, parent_path());
    PRINT_OF(p1, stem());
    PRINT_OF(p1, extension());

    PRINT_OF(p1, replace_extension("new"));
    char buf[] = "hello";
    PRINT_OF(p1, append(buf, buf + sizeof(buf)));
    PRINT_OF(p1, remove_filename());
}

ADD_TEST_F(BoostTest, Test_Bind);
ADD_TEST_F(BoostTest, Test_Ptr);
ADD_TEST_F(BoostTest, Test_Locale);
ADD_TEST_F(BoostTest, Test_CircularBuffer_int);
ADD_TEST_F(BoostTest, Test_CircularBuffer_char);
ADD_TEST_F(BoostTest, Test_CircularBuffer_ReloadSize);
ADD_TEST_F(BoostTest, Test_Split);
ADD_TEST_F(BoostTest, Test_Path1);
ADD_TEST_F(BoostTest, Test_Path2);
