#include "char_map_test.h"
#include <unordered_map>

CharMapTest::CharMapTest()
{

}

CharMapTest::~CharMapTest()
{

}

namespace char_map_test
{
// 默认的map<char *,int>的key是指针，比较的是指针值的大小，不能进行字符串的匹配。
// 可以编写字符串的比较类来实现map以char*为key，代码如下：
struct MapCharKeyCmp
{
    bool operator()(const char* str1, const char* str2)
    {
        return strcmp(str1, str2) < 0;
    }
};
}

void CharMapTest::CharMapTest001()
{
    typedef std::map<char*, int, char_map_test::MapCharKeyCmp> CStrMap; // 如果char*指向的是局部变量，可能会出现随机结果。务必管理好char*的生命周期！
    CStrMap a;
}

namespace char_map_test
{
// 使用const char*字符串作为unordered_map的key，需要做两件事，一是重载比较函数Pred，二是重载Hash函数。
// 重载比较函数的原因是，如果使用它本身的比较函数std::equal_to<const char*>的话，它比较的仅仅只是两个指针的地址是否相同，而不是两个指针指向的字符串是否相同。
// 重载哈希函数的原因也类似，因为如果使用hash<const char*>的话，存储时确实能正确地构造出一个哈希列表，但是它是用指针地址作为key值进行哈希的，而不是指针指向的字符串进行哈希。
struct hash_map_char_cmp
{
    bool operator()(const char* s1, const char* s2) const
    {
        return strcmp(s1, s2) == 0;
    }
};

struct char_hash_func
{
    size_t operator()(const char* str) const
    {
        int seed = 131;
        int hash = 0;

        while (*str)
        {
            hash = (hash * seed) + (*str);
            str++;
        }

        return (hash & 0x7FFFFFFF);
    }
};
}

void CharMapTest::CharHashMapTest001()
{
    typedef std::unordered_map<const char*, int, char_map_test::char_hash_func, char_map_test::hash_map_char_cmp> CStrHashMap;
    CStrHashMap a;
}

ADD_TEST_F(CharMapTest, CharMapTest001);
ADD_TEST_F(CharMapTest, CharHashMapTest001);
