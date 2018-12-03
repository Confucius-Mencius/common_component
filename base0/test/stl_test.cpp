#include "stl_test.h"
#include <list>
#include "hash_container.h"
#include <queue>

// stl网站：http://stl123.net/

#include <sys/time.h>
#include "misc_util.h"

using namespace std;

STLTest::STLTest()
{

}

STLTest::~STLTest()
{

}

void STLTest::Test_Vector()
{
    typedef std::vector<int> MyVec;
    MyVec my_vec;

    for (int i = 0; i < 10; ++i)
    {
        my_vec.push_back(i);
    }

    for (int i = 0; i < 10; ++i)
    {
        my_vec.push_back(i + 1);
    }

    for (MyVec::const_iterator it = my_vec.begin(); it != my_vec.end(); ++it)
    {
        std::cout << *it << " ";
    }

    std::cout << std::endl << "size_: " << my_vec.size() << std::endl;
    EXPECT_EQ(20, (int) my_vec.size());

    for (MyVec::iterator it = my_vec.begin(); it != my_vec.end();)
    {
        if (5 == *it)
        {
            // do something here

            it = my_vec.erase(it);
        }
        else
        {
            ++it;
        }
    }

    for (MyVec::const_iterator it = my_vec.begin(); it != my_vec.end(); ++it)
    {
        std::cout << *it << " ";
    }

    std::cout << std::endl << "size_: " << my_vec.size() << std::endl;
    EXPECT_EQ(18, (int) my_vec.size());
}

void STLTest::Test_List()
{
    typedef std::list<int> MyList;
    MyList my_list;

    for (int i = 0; i < 10; ++i)
    {
        my_list.push_back(i);
    }

    for (int i = 0; i < 10; ++i)
    {
        my_list.push_back(i + 1);
    }

    for (MyList::const_iterator it = my_list.begin(); it != my_list.end(); ++it)
    {
        std::cout << *it << " ";
    }

    std::cout << std::endl << "size_: " << my_list.size() << std::endl;
    EXPECT_EQ(20, (int) my_list.size());

    for (MyList::iterator it = my_list.begin(); it != my_list.end();)
    {
        if (5 == *it)
        {
            // do something here

            it = my_list.erase(it);
        }
        else
        {
            ++it;
        }
    }

    for (MyList::const_iterator it = my_list.begin(); it != my_list.end(); ++it)
    {
        std::cout << *it << " ";
    }

    std::cout << std::endl << "size_: " << my_list.size() << std::endl;
    EXPECT_EQ(18, (int) my_list.size());
}

void STLTest::Test_Map()
{
    typedef std::map<int, int> MyMap;
    MyMap my_map;

    for (int i = 0; i < 10; ++i)
    {
        EXPECT_TRUE(my_map.insert(MyMap::value_type(i, i)).second);
    }

    for (int i = 0; i < 10; ++i)
    {
        EXPECT_FALSE(my_map.insert(MyMap::value_type(i, i + 1)).second); // key已经存在，插入失败
    }

    // 遍历是有序的
    for (MyMap::const_iterator it = my_map.begin(); it != my_map.end(); ++it)
    {
        std::cout << it->first << "<->" << it->second << " ";
    }

    std::cout << std::endl << "size_: " << my_map.size() << std::endl;
    EXPECT_EQ(10, (int) my_map.size());

    for (MyMap::iterator it = my_map.begin(); it != my_map.end();)
    {
        if (5 == it->first) // 根据主键做删除判断
        {
            // do something here

            my_map.erase(it++);
        }
        else
        {
            ++it;
        }
    }

    for (MyMap::const_iterator it = my_map.begin(); it != my_map.end(); ++it)
    {
        std::cout << it->first << "<->" << it->second << " ";
    }

    std::cout << std::endl << "size_: " << my_map.size() << std::endl;
    EXPECT_EQ(9, (int) my_map.size());
}

void STLTest::Test_Set()
{
    typedef std::set<int> MySet;
    MySet my_set;

    for (int i = 0; i < 10; ++i)
    {
        EXPECT_TRUE(my_set.insert(i).second);
    }

    for (int i = 0; i < 10; ++i)
    {
        if (i < 9)
        {
            EXPECT_FALSE(my_set.insert(i + 1).second); // key已经存在，插入失败
        }
        else
        {
            EXPECT_TRUE(my_set.insert(i + 1).second);
        }
    }

    // 遍历是有序的
    for (MySet::const_iterator it = my_set.begin(); it != my_set.end(); ++it)
    {
        std::cout << *it << " ";
    }

    std::cout << std::endl << "size_: " << my_set.size() << std::endl;
    EXPECT_EQ(11, (int) my_set.size());

    for (MySet::iterator it = my_set.begin(); it != my_set.end();)
    {
        if (5 == *it)
        {
            // do something here

            my_set.erase(it++);
        }
        else
        {
            ++it;
        }
    }

    for (MySet::const_iterator it = my_set.begin(); it != my_set.end(); ++it)
    {
        std::cout << *it << " ";
    }

    std::cout << std::endl << "size_: " << my_set.size() << std::endl;
    EXPECT_EQ(10, (int) my_set.size());
}

void STLTest::Test_Multimap()
{
    typedef std::multimap<int, int> MyMultimap;
    MyMultimap my_multimap;

    for (int i = 0; i < 10; ++i)
    {
        my_multimap.insert(MyMultimap::value_type(i, i));
    }

    for (int i = 0; i < 10; ++i)
    {
        my_multimap.insert(MyMultimap::value_type(i, i + 1));
    }

    // 遍历是有序的
    for (MyMultimap::const_iterator it = my_multimap.begin(); it != my_multimap.end(); ++it)
    {
        std::cout << it->first << "<->" << it->second << " ";
    }

    std::cout << std::endl << "size_: " << my_multimap.size() << std::endl;
    EXPECT_EQ(20, (int) my_multimap.size());

    for (MyMultimap::iterator it = my_multimap.begin(); it != my_multimap.end();)
    {
        if (5 == it->first)  // 根据主键做删除判断
        {
            // do something here

            my_multimap.erase(it++);
        }
        else
        {
            ++it;
        }
    }

    for (MyMultimap::const_iterator it = my_multimap.begin(); it != my_multimap.end(); ++it)
    {
        std::cout << it->first << "<->" << it->second << " ";
    }

    std::cout << std::endl << "size_: " << my_multimap.size() << std::endl;
    EXPECT_EQ(18, (int) my_multimap.size());

    // TODO equal_range,lower_bound,upper_bound测试
}

void STLTest::Test_Multiset()
{
    typedef std::multiset<int> MyMultiset;
    MyMultiset my_multiset;

    for (int i = 0; i < 10; ++i)
    {
        my_multiset.insert(i);
    }

    for (int i = 0; i < 10; ++i)
    {
        my_multiset.insert(i + 1);
    }

    // 遍历是有序的
    for (MyMultiset::const_iterator it = my_multiset.begin(); it != my_multiset.end(); ++it)
    {
        std::cout << *it << " ";
    }

    std::cout << std::endl << "size_: " << my_multiset.size() << std::endl;
    EXPECT_EQ(20, (int) my_multiset.size());

    for (MyMultiset::iterator it = my_multiset.begin(); it != my_multiset.end();)
    {
        if (5 == *it)
        {
            // do something here

            my_multiset.erase(it++);
        }
        else
        {
            ++it;
        }
    }

    for (MyMultiset::const_iterator it = my_multiset.begin(); it != my_multiset.end(); ++it)
    {
        std::cout << *it << " ";
    }

    std::cout << std::endl << "size_: " << my_multiset.size() << std::endl;
    EXPECT_EQ(18, (int) my_multiset.size());

    // TODO equal_range,lower_bound,upper_bound测试
}

void STLTest::Test_HashMap()
{
    typedef __hash_map<int, int> MyHashMap;
    MyHashMap my_hash_map;

    for (int i = 0; i < 10; ++i)
    {
        EXPECT_TRUE(my_hash_map.insert(MyHashMap::value_type(i, i)).second);
    }

    for (int i = 0; i < 10; ++i)
    {
        EXPECT_FALSE(my_hash_map.insert(MyHashMap::value_type(i, i + 1)).second); // key已经存在，插入失败
    }

    // 遍历是无序的
    for (MyHashMap::const_iterator it = my_hash_map.begin(); it != my_hash_map.end(); ++it)
    {
        std::cout << it->first << "<->" << it->second << " ";
    }

    std::cout << std::endl << "size_: " << my_hash_map.size() << std::endl;
    EXPECT_EQ(10, (int) my_hash_map.size());

    for (MyHashMap::iterator it = my_hash_map.begin(); it != my_hash_map.end();)
    {
        if (5 == it->first) // 根据主键做删除判断
        {
            // do something here

            it = my_hash_map.erase(it);
        }
        else
        {
            ++it;
        }
    }

    for (MyHashMap::const_iterator it = my_hash_map.begin(); it != my_hash_map.end(); ++it)
    {
        std::cout << it->first << "<->" << it->second << " ";
    }

    std::cout << std::endl << "size_: " << my_hash_map.size() << std::endl;
    EXPECT_EQ(9, (int) my_hash_map.size());
}

void STLTest::Test_HashMultimap()
{
    typedef __hash_multimap<int, int> MyHashMultimap;
    MyHashMultimap my_hash_multimap;

    for (int i = 0; i < 10; ++i)
    {
        my_hash_multimap.insert(MyHashMultimap::value_type(i, i));
    }

    for (int i = 0; i < 10; ++i)
    {
        my_hash_multimap.insert(MyHashMultimap::value_type(i, i + 1)); // key已经存在，还能成功插入
    }

    // 遍历是无序的
    for (MyHashMultimap::const_iterator it = my_hash_multimap.begin(); it != my_hash_multimap.end(); ++it)
    {
        std::cout << it->first << "<->" << it->second << " ";
    }

    std::cout << std::endl << "size_: " << my_hash_multimap.size() << std::endl;
    EXPECT_EQ(20, (int) my_hash_multimap.size());

    for (MyHashMultimap::iterator it = my_hash_multimap.begin(); it != my_hash_multimap.end();)
    {
        if (5 == it->first) // 根据主键做删除判断
        {
            // do something here

            it = my_hash_multimap.erase(it);
        }
        else
        {
            ++it;
        }
    }

    for (MyHashMultimap::const_iterator it = my_hash_multimap.begin(); it != my_hash_multimap.end(); ++it)
    {
        std::cout << it->first << "<->" << it->second << " ";
    }

    std::cout << std::endl << "size_: " << my_hash_multimap.size() << std::endl;
    EXPECT_EQ(18, (int) my_hash_multimap.size());

    // TODO equal_range测试
}

void STLTest::Test_TraverseMap()
{
    typedef std::map<int, std::string> MyMap;
    MyMap my_map;
    my_map.insert(MyMap::value_type(3, "3"));
    my_map.insert(MyMap::value_type(1, "1"));
    my_map.insert(MyMap::value_type(2, "2"));

    // 遍历是有序的
    for (MyMap::iterator it = my_map.begin(); it != my_map.end(); ++it)
    {
        std::cout << it->first << std::endl;
    }

    for (MyMap::reverse_iterator it = my_map.rbegin(); it != my_map.rend(); ++it)
    {
        std::cout << it->first << std::endl;
    }
}

void STLTest::Test_Vector_end()
{
    std::vector<std::string> v;
    for (int i = 0; i < 10000; ++i)
    {
        v.push_back("hello");
        v.push_back("world");
    }

    for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it)
    {
        if (it != (v.end() - 1))
        {
            std::cout << "x" << std::endl;
        }
    }
};

ADD_TEST_F(STLTest, Test_Vector)

ADD_TEST_F(STLTest, Test_List);
ADD_TEST_F(STLTest, Test_Map);
ADD_TEST_F(STLTest, Test_Set);
ADD_TEST_F(STLTest, Test_Multimap);
ADD_TEST_F(STLTest, Test_Multiset);
ADD_TEST_F(STLTest, Test_HashMap);
ADD_TEST_F(STLTest, Test_HashMultimap);
ADD_TEST_F(STLTest, Test_TraverseMap);
ADD_TEST_F(STLTest, Test_Vector_end);

/*
 map缺省地是用less<Key>作为比较器，所以它要求作为Key的类要重载“<”操作符。如果此时，没有重载“<”操作符，而是重载了“>”操作符自然就会报错。
 如果用greater<Key>作为比较器，就必要重载Key类中的“>”操作符。
 */

namespace complex_key_test
{
/**
 * 结构体作为map的key或放入set中，需要重载'<'运算符或'>'运算符。
 * map缺省是用less<Key>作为比较器，即按key升序排列，它要求作为Key的类型必须重载'<'运算符，
 * 如果没有重载'<'运算符，就算重载了'>'运算符也会报错。相反，也可以显式地用greater<Key>作为比较器，
 * 表示按key降序排列，此时就必须重载'>'运算符了。
 */
struct Key
{
    int type;
    int scale;

    Key(int type, int scale)
    {
        this->type = type;
        this->scale = scale;
    }

    // 升序排列（默认的）
    bool operator<(const Key& rhs) const
    {
        if (type != rhs.type) // 类型按升序排列
        {
            return (type < rhs.type);
        }
        else // 如果类型相同，按比例尺升序排列
        {
            return (scale < rhs.scale);
        }
    }

    // 降序排列
    bool operator>(const Key& rhs) const
    {
        if (type != rhs.type) // 类型按降序排列
        {
            return (type > rhs.type);
        }
        else // 如果类型相同，按比例尺降序排列
        {
            return (scale > rhs.scale);
        }
    }
};

// 此结构体作为map的value
struct Value
{
    int num1;
    int num2;
};

void Test001()
{
    std::map<Key, Value, std::greater<Key> > tmpMap;

    Value v1 = {1, 1};
    tmpMap.insert(std::pair<Key, Value>(Key(1, 2), v1));

    Value v2 = {2, 1};
    tmpMap.insert(std::pair<Key, Value>(Key(2, 2), v2));

    Value v3 = {3, 1};
    tmpMap.insert(std::pair<Key, Value>(Key(3, 2), v3));

    for (std::map<Key, Value, std::greater<Key> >::iterator it =
        tmpMap.begin(); it != tmpMap.end(); ++it)
    {
        std::cout << it->second.num1 << std::endl;
    }
}

/*
 附：stl中map和set的声明，二者比较像，底层都是用红黑树实现的
 template < class Key, class Compare = less<Key>,
 class Allocator = allocator<Key> > class set;

 template < class Key, class T, class Compare = less<Key>,
 class Allocator = allocator<pair<const Key,T> > > class map;

 template < class Key, class Compare = less<Key>,
 class Allocator = allocator<Key> > class multiset;

 template < class Key, class T, class Compare = less<Key>,
 class Allocator = allocator<pair<const Key,T> > > class multimap;
 从上面的声明可以看出，也可以定义一个函数对象Compare，声明map或set类型时传进入，如：
 struct TTimeCompare
 {
 bool operator ()(const CTimerEvent* e1, const CTimerEvent* e2) const
 {
 return (e1->m_oNextTick < e2->m_oNextTick);
 }
 };

 typedef std::multiset<CTimerEvent*, TTimeCompare> TEventSet;

 struct ltstr // less than
 {
 bool operator()(const char* s1, const char* s2) const
 {
 return strcmp(s1, s2) < 0;
 }
 };

 std::set<const char*, ltstr> tmpSet; // set<Key, Compare, Alloc>
 std::map<const char*, int, ltstr> tmpMap; // map<Key, Data, Compare, Alloc>

 struct eqstr // equal
 {
 bool operator()(const char* s1, const char* s2) const
 {
 return strcmp(s1, s2) == 0;
 }
 };

 __hash_map<const char*, int, hash<const char*>, eqstr> stHashMap; // __hash_map<Key, Data, HashFcn, EqualKey, AllocFromShm>

 // 自定义hash函数
 namespace std
 {
 template<>
 struct hash<KEY_TYPE>
 {
 size_t operator()(const KEY_TYPE& key) const
 {
 return key.Hash();
 }
 };
 }
 */

ADD_TEST(ComplexKeyTest, Test001);
} /* namespace complex_key_test */

/*
 STL中的排序
 0 前言: STL，为什么你必须掌握
 对于程序员来说，数据结构是必修的一门课。从查找到排序，从链表到二叉树，几乎所有的算法和原理都需要理解，理解不了也要死记硬背下来。幸运的是这些理论 都已经比较成熟，算法也基本固定下来，不需要你再去花费心思去考虑其算法原理，也不用再去验证其准确性。不过，等你开始应用计算机语言来工作的时候，你会 发现，面对不同的需求你需要一次又一次去用代码重复实现这些已经成熟的算法，而且会一次又一次陷入一些由于自己疏忽而产生的bug中。这时，你想找一种工 具，已经帮你实现这些功能，你想怎么用就怎么用，同时不影响性能。你需要的就是STL, 标准模板库！
 西方有句谚语：不要重复发明轮子！
 STL几乎封装了所有的数据结构中的算法，从链表到队列，从向量到堆栈，对hash到二叉树，从搜索到排序，从增加到删除......可以说，如果你理解了STL，你会发现你已不用拘泥于算法本身，从而站在巨人的肩膀上去考虑更高级的应用。
 排序是最广泛的算法之一，本文详细介绍了STL中不同排序算法的用法和区别。
 1 STL提供的Sort 算法
 C++之所以得到这么多人的喜欢，是因为它既具有面向对象的概念，又保持了C语言高效的特点。STL 排序算法同样需要保持高效。因此，对于不同的需求，STL提供的不同的函数，不同的函数，实现的算法又不尽相同。
 1.1 所有sort算法介绍
 所有的sort算法的参数都需要输入一 个范围，[begin, end)。这里使用的迭代器(iterator)都需是随机迭代器(RadomAccessIterator), 也就是说可以随机访问的迭代器，如：it+n什么的。（partition 和stable_partition 除外）
 如果你需要自己定义比较函数，你可以把你定义好的仿函数(functor)作为参数传入。每种算法都支持传入比较函数。以下是所有STL sort算法函数的名字列表:
 函数名 功能描述
 sort    对给定区间所有元素进行排序
 stable_sort 对给定区间所有元素进行稳定排序
 partial_sort    对给定区间所有元素部分排序
 partial_sort_copy   对给定区间复制并排序
 nth_element 找出给定区间的某个位置对应的元素
 is_sorted   判断一个区间是否已经排好序
 partition   使得符合某个条件的元素放在前面
 stable_partition    相对稳定的使得符合某个条件的元素放在前面
 其中nth_element 是最不易理解的，实际上，这个函数是用来找出第几个。例如：找出包含7个元素的数组中排在中间那个数的值，此时，我可能不关心前面，也不关心后面，我只关心排在第四位的元素值是多少。
 1.2 sort 中的比较函数
 当你需要按照某种特定方式进行排序时，你需要给sort指定比较函数，否则程序会自动提供给你一个比较函数。
 vector < int > vect;
 //...
 sort(vect.begin(), vect.end());
 //此时相当于调用
 sort(vect.begin(), vect.end(), less<int>() );

 上述例子中系统自己为sort提供了less仿函数。在STL中还提供了其他仿函数，以下是仿函数列表:
 名称  功能描述
 equal_to    相等
 not_equal_to    不相等
 less    小于
 greater 大于
 less_equal  小于等于
 greater_equal   大于等于
 需要注意的是，这些函数不是都能适用于你的sort算法，如何选择，决定于你的应用。另外，不能直接写入仿函数的名字，而是要写其重载的()函数：
 less<int>()
 greater<int>()
 当你的容器中元素时一些标准类型（int float char)或者string时，你可以直接使用这些函数模板。但如果你时自己定义的类型或者你需要按照其他方式排序，你可以有两种方法来达到效果：一种是自己写比较函数。另一种是重载类型的'<'操作赋。
 #include <iostream>
 #include <algorithm>
 #include <functional>
 #include <vector>
 using namespace std;

 class myclass {
 public:
 myclass(int a, int b):first(a), second(b){}
 int first;
 int second;
 bool operator < (const myclass &m)const {
 return first < m.first;
 }
 };

 bool less_second(const myclass & m1, const myclass & m2) {
 return m1.second < m2.second;
 }

 int main() {

 vector< myclass > vect;
 for(int i = 0 ; i < 10 ; i ++){
 myclass my(10-i, i*3);
 vect.push_back(my);
 }
 for(int i = 0 ; i < vect.size_(); i ++)
 cout<<"("<<vect[i].first<<","<<vect[i].second<<")/n";
 sort(vect.begin(), vect.end());
 cout<<"after sorted by first:"<<endl;
 for(int i = 0 ; i < vect.size_(); i ++)
 cout<<"("<<vect[i].first<<","<<vect[i].second<<")/n";
 cout<<"after sorted by second:"<<endl;
 sort(vect.begin(), vect.end(), less_second);
 for(int i = 0 ; i < vect.size_(); i ++)
 cout<<"("<<vect[i].first<<","<<vect[i].second<<")/n";

 return 0 ;
 }

 知道其输出结果是什么了吧：
 (10,0)
 (9,3)
 (8,6)
 (7,9)
 (6,12)
 (5,15)
 (4,18)
 (3,21)
 (2,24)
 (1,27)
 after sorted by first:
 (1,27)
 (2,24)
 (3,21)
 (4,18)
 (5,15)
 (6,12)
 (7,9)
 (8,6)
 (9,3)
 (10,0)
 after sorted by second:
 (10,0)
 (9,3)
 (8,6)
 (7,9)
 (6,12)
 (5,15)
 (4,18)
 (3,21)
 (2,24)
 (1,27)
 1.3 sort 的稳定性
 你发现有sort和 stable_sort，还有 partition 和stable_partition， 感到奇怪吧。其中的区别是，带有stable的函数可保证相等元素的原本相对次序在排序后保持不变。或许你会问，既然相等，你还管他相对位置呢，也分不清 楚谁是谁了？这里需要弄清楚一个问题，这里的相等，是指你提供的函数表示两个元素相等，并不一定是一摸一样的元素。
 例如，如果你写一个比较函数:
 bool less_len(const string &str1, const string &str2)
 {
 return str1.length() < str2.length();
 }

 此 时，"apple" 和 "winter" 就是相等的，如果在"apple" 出现在"winter"前面，用带stable的函数排序后，他们的次序一定不变，如果你使用的是不带"stable"的函数排序，那么排序完后， "Winter"有可能在"apple"的前面。
 1.4 全排序
 全排序即把所给定范围所有的元素按照大小关系顺序排列。用于全排序的函数有
 template <class RandomAccessIterator>
 void sort(RandomAccessIterator first, RandomAccessIterator last);

 template <class RandomAccessIterator, class StrictWeakOrdering>
 void sort(RandomAccessIterator first, RandomAccessIterator last,
 StrictWeakOrdering comp);

 template <class RandomAccessIterator>
 void stable_sort(RandomAccessIterator first, RandomAccessIterator last);

 template <class RandomAccessIterator, class StrictWeakOrdering>
 void stable_sort(RandomAccessIterator first, RandomAccessIterator last,
 StrictWeakOrdering comp);

 在 第1，3种形式中，sort 和 stable_sort都没有指定比较函数，系统会默认使用operator< 对区间[first,last)内的所有元素进行排序, 因此，如果你使用的类型义军已经重载了operator<函数，那么你可以省心了。第2, 4种形式，你可以随意指定比较函数，应用更为灵活一些。来看看实际应用：
 班上有10个学生，我想知道他们的成绩排名。
 #include <iostream>
 #include <algorithm>
 #include <functional>
 #include <vector>
 #include <string>
 using namespace std;

 class student{
 public:
 student(const string &a, int b):name(a), score(b){}
 string name;
 int score;
 bool operator < (const student &m)const {
 return score< m.score;
 }
 };

 int main() {
 vector< student> vect;
 student st1("Tom", 74);
 vect.push_back(st1);
 st1.name="Jimy";
 st1.score=56;
 vect.push_back(st1);
 st1.name="Mary";
 st1.score=92;
 vect.push_back(st1);
 st1.name="Jessy";
 st1.score=85;
 vect.push_back(st1);
 st1.name="Jone";
 st1.score=56;
 vect.push_back(st1);
 st1.name="Bush";
 st1.score=52;
 vect.push_back(st1);
 st1.name="Winter";
 st1.score=77;
 vect.push_back(st1);
 st1.name="Andyer";
 st1.score=63;
 vect.push_back(st1);
 st1.name="Lily";
 st1.score=76;
 vect.push_back(st1);
 st1.name="Maryia";
 st1.score=89;
 vect.push_back(st1);
 cout<<"------before Sort..."<<endl;
 for(int i = 0 ; i < vect.size_(); i ++) cout<<vect[i].name<<":/t"<<vect[i].score<<endl;
 stable_sort(vect.begin(), vect.end(),less<student>());
 cout <<"-----after sort ...."<<endl;
 for(int i = 0 ; i < vect.size_(); i ++) cout<<vect[i].name<<":/t"<<vect[i].score<<endl;
 return 0 ;
 }

 其输出是：
 ------before sort...
 Tom:    74
 Jimy:   56
 Mary:   92
 Jessy:  85
 Jone:   56
 Bush:   52
 Winter: 77
 Andyer: 63
 Lily:   76
 Maryia: 89
 -----after sort ....
 Bush:   52
 Jimy:   56
 Jone:   56
 Andyer: 63
 Tom:    74
 Lily:   76
 Winter: 77
 Jessy:  85
 Maryia: 89
 Mary:   92
 sort采用的是成熟的"快速排序算法"(目前大部分STL版本已经不是采用简单的快速排序，而是结合内插排序算法)。注1， 可以保证很好的平均性能、复杂度为n*log(n)，由于单纯的快速排序在理论上有最差的情况，性能很低，其算法复杂度为n*n，但目前大部分的STL版 本都已经在这方面做了优化，因此你可以放心使用。stable_sort采用的是"归并排序"，分派足够内存是，其算法复杂度为n*log(n), 否则其复杂度为n*log(n)*log(n)，其优点是会保持相等元素之间的相对位置在排序前后保持一致。
 1.5 局部排序
 局部排序其实是为了减少不必要的操作而提供的排序方式。其函数原型为：
 template <class RandomAccessIterator>
 void partial_sort(RandomAccessIterator first,
 RandomAccessIterator middle,
 RandomAccessIterator last);

 template <class RandomAccessIterator, class StrictWeakOrdering>
 void partial_sort(RandomAccessIterator first,
 RandomAccessIterator middle,
 RandomAccessIterator last,
 StrictWeakOrdering comp);

 template <class InputIterator, class RandomAccessIterator>
 RandomAccessIterator partial_sort_copy(InputIterator first, InputIterator last,
 RandomAccessIterator result_first,
 RandomAccessIterator result_last);

 template <class InputIterator, class RandomAccessIterator,
 class StrictWeakOrdering>
 RandomAccessIterator partial_sort_copy(InputIterator first, InputIterator last,
 RandomAccessIterator result_first,
 RandomAccessIterator result_last, Compare comp);

 理解了sort 和stable_sort后，再来理解partial_sort 就比较容易了。先看看其用途: 班上有10个学生，我想知道分数最低的5名是哪些人。如果没有partial_sort，你就需要用sort把所有人排好序，然后再取前5个。现在你只需要对分数最低5名排序，把上面的程序做如下修改：
 stable_sort(vect.begin(), vect.end(),less<student>());
 替换为：
 partial_sort(vect.begin(), vect.begin()+5, vect.end(),less<student>());

 输出结果为：
 ------before sort...
 Tom:    74
 Jimy:   56
 Mary:   92
 Jessy:  85
 Jone:   56
 Bush:   52
 Winter: 77
 Andyer: 63
 Lily:   76
 Maryia: 89
 -----after sort ....
 Bush:   52
 Jimy:   56
 Jone:   56
 Andyer: 63
 Tom:    74
 Mary:   92
 Jessy:  85
 Winter: 77
 Lily:   76
 Maryia: 89
 这样的好处知道了吗？当数据量小的时候可能看不出优势，如果是100万学生，我想找分数最少的5个人......
 partial_sort采用的堆排序（heapsort），它在任何情况下的复杂度都是n*log(n). 如果你希望用partial_sort来实现全排序，你只要让middle=last就可以了。
 partial_sort_copy其实是copy和partial_sort的组合。被排序(被复制)的数量是[first, last)和[result_first, result_last)中区间较小的那个。如果[result_first, result_last)区间大于[first, last)区间，那么partial_sort相当于copy和sort的组合。
 1.6 nth_element 指定元素排序
 nth_element一个容易看懂但解释比较麻烦的排序。用例子说会更方便：
 班上有10个学生，我想知道分数排在倒数第4名的学生。
 如果要满足上述需求，可以用sort排好序，然后取第4位（因为是由小到大排), 更聪明的朋友会用partial_sort, 只排前4位，然后得到第4位。其实这是你还是浪费，因为前两位你根本没有必要排序，此时，你就需要nth_element:
 template <class RandomAccessIterator>
 void nth_element(RandomAccessIterator first, RandomAccessIterator nth,
 RandomAccessIterator last);

 template <class RandomAccessIterator, class StrictWeakOrdering>
 void nth_element(RandomAccessIterator first, RandomAccessIterator nth,
 RandomAccessIterator last, StrictWeakOrdering comp);

 对于上述实例需求，你只需要按下面要求修改1.4中的程序：
 stable_sort(vect.begin(), vect.end(),less<student>());
 替换为：
 nth_element(vect.begin(), vect.begin()+3, vect.end(),less<student>());

 运行结果为：
 ------before sort...
 Tom:    74
 Jimy:   56
 Mary:   92
 Jessy:  85
 Jone:   56
 Bush:   52
 Winter: 77
 Andyer: 63
 Lily:   76
 Maryia: 89
 -----after sort ....
 Jone:   56
 Bush:   52
 Jimy:   56
 Andyer: 63
 Jessy:  85
 Mary:   92
 Winter: 77
 Tom:    74
 Lily:   76
 Maryia: 89
 第四个是谁？Andyer，这个倒霉的家伙。为什么是begin()+3而不是+4? 我开始写这篇文章的时候也没有在意，后来在ilovevc 的提醒下，发现了这个问题。begin()是第一个，begin()+1是第二个，... begin()+3当然就是第四个了。
 1.7 partition 和stable_partition
 好像这两个函数并不是用来排序的，'分类'算法，会更加贴切一些。partition就是把一个区间中的元素按照某个条件分成两类。其函数原型为：
 template <class ForwardIterator, class Predicate>
 ForwardIterator partition(ForwardIterator first,
 ForwardIterator last, Predicate pred)
 template <class ForwardIterator, class Predicate>
 ForwardIterator stable_partition(ForwardIterator first, ForwardIterator last,
 Predicate pred);

 看看应用吧： 班上10个学生，计算所有没有及格（低于60分）的学生。 你只需要按照下面格式替换1.4中的程序：
 stable_sort(vect.begin(), vect.end(),less<student>());
 替换为：
 student exam("pass", 60);
 stable_partition(vect.begin(), vect.end(), bind2nd(less<student>(), exam));

 其输出结果为：
 ------before sort...
 Tom:    74
 Jimy:   56
 Mary:   92
 Jessy:  85
 Jone:   56
 Bush:   52
 Winter: 77
 Andyer: 63
 Lily:   76
 Maryia: 89
 -----after sort ....
 Jimy:   56
 Jone:   56
 Bush:   52
 Tom:    74
 Mary:   92
 Jessy:  85
 Winter: 77
 Andyer: 63
 Lily:   76
 Maryia: 89
 看见了吗，Jimy，Jone, Bush(难怪说美国总统比较笨  )都没有及格。而且使用的是stable_partition, 元素之间的相对次序是没有变.
 2 Sort 和容器
 STL中标准容器主要vector, list, deque, string, set, multiset, map, multimay， 其中set, multiset, map, multimap都是以树结构的方式存储其元素详细内容请参看：学习STL map, STL set之数据结构基础. 因此在这些容器中，元素一直是有序的。
 这些容器的迭代器类型并不是随机型迭代器，因此，上述的那些排序函数，对于这些容器是不可用的。上述sort函数对于下列容器是可用的：
 vector
 string
 deque
 如果你自己定义的容器也支持随机型迭代器，那么使用排序算法是没有任何问题的。
 对于list容器，list自带一个sort成员函数list::sort(). 它和算法函数中的sort差不多，但是list::sort是基于指针的方式排序，也就是说，所有的数据移动和比较都是此用指针的方式实现，因此排序后的 迭代器一直保持有效（vector中sort后的迭代器会失效).
 3 选择合适的排序函数
 为什么要选择合适的排序函数？可能你并不关心效率(这里的效率指的是程序运行时间), 或者说你的数据量很小， 因此你觉得随便用哪个函数都无关紧要。
 其实不然，即使你不关心效率，如果你选择合适的排序函数，你会让你的代码更容易让人明白，你会让你的代码更有扩充性，逐渐养成一个良好的习惯，很重要吧  。
 如果你以前有用过C语言中的qsort, 想知道qsort和他们的比较，那我告诉你，qsort和sort是一样的，因为他们采用的都是快速排序。从效率上看，以下几种sort算法的是一个排序，效率由高到低（耗时由小变大）：
 partion
 stable_partition
 nth_element
 partial_sort
 sort
 stable_sort
 记得，以前翻译过Effective STL的文章，其中对如何选择排序函数总结的很好：
 若需对vector, string, deque, 或 array容器进行全排序，你可选择sort或stable_sort；
 若只需对vector, string, deque, 或 array容器中取得top n的元素，部分排序partial_sort是首选.
 若对于vector, string, deque, 或array容器，你需要找到第n个位置的元素或者你需要得到top n且不关系top n中的内部顺序，nth_element是最理想的；
 若你需要从标准序列容器或者array中把满足某个条件或者不满足某个条件的元素分开，你最好使用partition或stable_partition；
 若使用的list容器，你可以直接使用partition和stable_partition算法，你可以使用list::sort代替sort和 stable_sort排序。若你需要得到partial_sort或nth_element的排序效果，你必须间接使用。正如上面介绍的有几种方式可以 选择。
 总之记住一句话： 如果你想节约时间，不要走弯路, 也不要走多余的路!
 4 小结
 讨论技术就像个无底洞，经常容易由一点可以引申另外无数个技术点。因此需要从全局的角度来观察问题，就像观察STL中的sort算法一样。其实在 STL还有make_heap, sort_heap等排序算法。本文章没有提到。本文以实例的方式，解释了STL中排序算法的特性，并总结了在实际情况下应如何选择合适的算法。
 */

namespace stl_test
{
#define number_count_128M (1U << 27) // 4字节整数的个数，=27时个数为128M，这里的1M=1024*1024。（这个数字视你自己的内存而定）
#define number_count_32M (1U << 25)
#define number_count_16M (1U << 24)

struct timeval g_tvStart;
struct timeval g_tvEnd;

/*
 红黑树：map、set
 */

void GetCurTime(struct timeval& rtv)
{
    gettimeofday(&rtv, NULL);
    //cout << "gettimeofday, tv_sec: " << rtv.tv_sec << ", tv_usec: " << rtv.tv_usec << endl;
}

void PrintTimeDiffMicroSeconds(const struct timeval& rtvBegin,
                               const struct timeval& rtvEnd)
{
    suseconds_t stMicroSeconds = 1000000 * (rtvEnd.tv_sec - rtvBegin.tv_sec)
                                 + (rtvEnd.tv_usec - rtvBegin.tv_usec);
    cout << stMicroSeconds << endl;
}

void PrintTimeDiffSeconds(const struct timeval& rtvBegin,
                          const struct timeval& rtvEnd)
{
    suseconds_t stMicroSeconds = 1000000 * (rtvEnd.tv_sec - rtvBegin.tv_sec)
                                 + (rtvEnd.tv_usec - rtvBegin.tv_usec);
    time_t stSeconds = stMicroSeconds / 1000000;
    cout << stSeconds << endl;
}

void PrintTimeDiff(const struct timeval& rtvBegin, const struct timeval& rtvEnd)
{
    // 秒，微妙两段
    time_t stSeconds = rtvEnd.tv_sec - rtvBegin.tv_sec;
    suseconds_t stMicroSeconds = rtvEnd.tv_usec - rtvBegin.tv_usec;
    cout << "seconds: " << stSeconds << ", micro seconds: " << stMicroSeconds
        << endl;
}

void PrintMemUsed(pid_t stPID, const char* pszProcessName)
{
    stringstream ss;

    ss.clear(); // 清除流标志（出错标志等）
    ss.str(""); // 清除内容
    //ss << "ps -e -o 'pid,comm,args,pcpu,rsz,vsz,stime,user,uid' | grep " << stPID << " | grep -v grep"; // 其中rsz是是实际内存（以KB为单位）
    ss << "ps -e -o 'rsz,vsz' | grep " << stPID << " | grep -v grep"; // 其中rsz是是实际内存（以KB为单位）
    const char* pszCmd = ss.str().c_str();
    //cout << pszCmd << endl;
    int ret = system(pszCmd);
    NOT_USED(ret);
}

void VectorTest(pid_t stPID, const char* pszProcessName)
{
    /*
     容量是指在容器下一次需要增长自己之前能够被加入到容器中的元素的总数，容量只与连续存储的容器相关，例如vector deque 或string ，list 不要求容量。为了知道一个容器的容量我们调用它的capacity()操作而长度size 是指容器当前拥有元素的个数为了获得容器的当前长度我们调用它的size()操作例。
     一个用于管理vector增长的简单而高效的策略。

     大小和容量

     要想搞清楚vector类的工作机制，首先要清楚它并不仅仅是一块内存。相反，每一个vector都关联有两个“尺寸”：一个称为 大小（size_），表示vector容纳的元素的数量；另一个称为容量（capacity），表示可被用来存储元素的内存总量。比方说，假如v是一个 vector，那么v.size_()和v.capacity()则分别返回v的 大小和容量。你可以想象一个vector看起来如下：

     当然了，在vector尾部留有额外的内存的用意在于，当使用push_back向vector追加元素时无需分配更多的内存。如果邻接于 vector尾部的内存当时恰好未被占用，那么vector的增长只要将那块内存合并过来即可。然而这样的好运气极其罕见，大多数情况下需要分配新的内 存，然后将vector现有的元素拷贝到那块内存中，然后销毁原来的元素，最后归还元素先前占用的内存。在vector中留有额外的内存的好处在于，这样 的重新分配（代价可能很昂贵）不会每当试图向vector追加一个元素时都发生。

     重新分配内存的代价有多高昂？它涉及如下四个步骤：

     为需要的新容量分配足够的内存；
     将元素从原来的内存拷贝到新内存中；
     销毁原来的内存中的元素；
     归还原来的内存。

     如果元素的数目为n，那么我们知道步骤(2)和(3)都要占用O(n)的时间，除非分配或归还内存的代价的增长超过O(n)，否则这两步将在全部运 行时间中占居支配地位。因此我们可以得出结论：无论用于重新分配的容量（capacity）是多少，重新分配一个 大小（size_）为n的vector需要占用O(n)的时间。

     这个结论暗示了一种折衷权衡。假如在重新分配时请求大量的额外内存，那么在相当长的时间内将无需再次进行重新分配，因此总体重新分配操作消耗的时间 相对较少，这种策略的代价在于将会浪费大量的空间。另一方面，我们可以只请求一点点额外的内存，这么做将会节约空间，但后继的重新分配操作将会耗费时间。 换句话说，我们面临一个经典的抉择：拿时间换空间，或者相反。

     重新分配策略

     作为一个极端的例子，假定每当填充vector一次我们就将其容量增加1个单位，这种策略耗费尽可能少的内存空间，但每当追加一个元素时都要重新分 配整个vector。我们说过，重新分配一个具有n个元素的vector占用O(n)的时间，因此，如果我们从一个空vector开始并将其增长到k个元 素，那么占用的总时间将会是O(1+2+...+k)或者O(k2)，这太可怕了！有没有更好的办法呢？

     比方说，假如不是以步幅1来增长vector的容量，而是以一个常量C的步幅来增长它将会如何？很明显这个策略将会减少重新分配的次数（基于因子C），所以这当然是一种改进，但这个改进到底有多大呢？

     理解这个改进的方式之一是要认识到此一新策略将针对每C个元素块进行一次重新分配。假设我们为总量为KxC个元素分配K块内存，那么，第一次重新分 配将会拷贝C个元素，第二次将会拷贝2xC个元素，等等。Big-O表示法不考虑常量因子，因此我们可以将所有的C因子分摊开来而获得 O(1+2+...+K)或者O(K2)的总时间。换句话说，时间仍然是元素个数的二次方程，不过是带有一个小得多的因子罢了。

     撇开较小的因子不谈，“二次行为”仍然太糟糕，即使有一个快速的处理器也是如此。实际上，对于快速的处理器来说尤其糟糕，因为快速的处理器通常伴有 大量的内存，而访问具有大量内存的快速处理器的程序员常常试图用尽那些内存（这是迟早的事）。这些程序员往往会发现，如果在运行一个二次算法的话，处理器 的速度于事无补。

     我们刚刚证实，一个希望能以小于“二次时间”而分配大型vector的实现是不能使用“每次填充时以常量步幅增长vector容量”的策略的，相 反，被分配的附加内存的数量必须随着vector的增长而增长。这个事实暗示存在一种简单的策略：vector从单个元素开始而后每当重新分配时倍增其容 量，如何？事实证明这种策略允许我们以O(n)的时间构建一个有着n个元素的vector。

     为了理解是如何获得这样的效率的，考虑当我们已经完全填满它并打算对其重新分配时的vector的状态：

     自最近一次重新分配内存以来被追加到vector中的元素有一半从未被拷贝过，而对于那些被拷贝的元素而言，其中一半只被拷贝了一次，其余的一半被拷贝了两次，以此类推。

     换句话说，有n/2的元素被拷贝了一次或多次，有n/4的元素被拷贝了两次或多次，等等。因此，拷贝元素的总数目为n/2 + n/4 +...，结果可以近似为n（随着n的增大，这个近似值越发精确）。撇开拷贝动作不谈，有n个元素被追加到了vector中，但操作占用的时间总量仍然是 O(n)而不是O(n2)。

     讨论

     C++标准并没有规定vector类必须以某种特定的方式管理其内存，它只是要求通过重复调用push_back而创建一个具有n个元素的vector耗费的时间不得超过O(n)，我们刚才讨论的策略可能是满足此项要求的最直截了当的一种。

     因为对于这样的操作来说vector具有优秀的时间性能，所以没有什么理由避免使用如下循环：

     vector<double> values;
     double x;
     while (cin >> x)
     values.push_back(x);

     是的，当其增长时，实现将会重新分配vector的元素，但是，如果我们事先能够预测vector最终 大小的话，这个重新分配耗费的时间将不会超过“一个常量因子”可能会占用的时间。

     练习

     1.设想我们通过以如下方式编写代码而努力使我们那个小型循环速度更快：

     while (cin >> x)
     {
     if (values.size_() == values.capacity())
     values.reserve(values.size_() + 1000);
     values.push_back(x);
     }

     效果将会如何？成员函数reserve进行一次重新分配，从而改变vector的capacity，使其大于或等于其参数。

     2.设想不是每次倍增vector的大小，而是增大三倍，在性能上将会产生什么样的影响？特别是，创建一个具有n个元素的vector的运行时间仍然为O(n)吗？

     3.设想你知道你的vector最终将拥有多少元素，在这种情况下，在填充元素之前你可以调用reserve来预先分配数量合适的内存。试一试你手 头的vector实现，看看调用reserve与否对你的程序的运行时间有多大的影响。
     */
    cout << "---vector test---" << endl;

    cout << "construct" << endl;
    GetCurTime(g_tvStart);
    vector<int> c(number_count_128M);
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "size_" << endl;
    GetCurTime(g_tvStart);
    c.size();
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "advance" << endl;
    GetCurTime(g_tvStart);
    vector<int>::iterator it1 = c.begin();
    advance(it1, number_count_128M / 2); // vector是随机访问容器
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "Find" << endl;
    GetCurTime(g_tvStart);
    vector<int>::iterator it2 = find(c.begin(), c.end(), 111111); // vector没有find函数
    if (it2 == c.end())
    {
        // 没找到
    }
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "erase" << endl;
    GetCurTime(g_tvStart);
    vector<int>::iterator it3 = c.erase(
        it2); // 一个成员被删除，会导致后面的成员进行copy和析构操作。erase函数要么删作指定位置loc的元素，要么删除区间[start, end)的所有元素，返回值是指向删除的最后一个元素的下一位置的迭代器
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "Insert" << endl;
    GetCurTime(g_tvStart);
    vector<int>::iterator it4 = c.insert(it3,
                                         111111); // 一个成员被插入，会导致后面的成员进行copy和析构操作。在指定位置前插入值为val的元素，返回指向这个元素的迭代器。也有批量插入的接口，无返回值
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "clear" << endl;
    GetCurTime(g_tvStart);
    c.clear(); // vector的clear不会释放内存，其他容器的clear会释放，见下面的测试（vector的clear成员只负责对其中每一个元素调用其析构函数，将vector的size置零，并不负责释放vector本身占用的内存空间）
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "swap" << endl;
    vector<int> c2(number_count_128M);
    GetCurTime(g_tvStart);
    c.swap(c2);
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);
    vector<int>().swap(c2);

    cout << "swap empty" << endl;
    GetCurTime(g_tvStart);
    vector<int>().swap(
        c); // vector()使用vector的默认构造函数构造一个临时vector对象，再在该临时对象上调用swap成员，swap调用之后对象c占用的空间就等于一个默认构造的对象的大小，而临时对象就具有原来对象c的大小，由于临时对象随即就会被析构，从而其占用的空间也随即被释放
    /* 或者如下所示 加一对大括号，意思一样的：
     {
     std::vector<int> tmp;
     c.swap(tmp);
     }
     加一对大括号是可以让tmp退出{}的时候自动析构 */
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    NOT_USED(it4);
}

void VectorEmplaceTest()
{
    {
//    emplace_back只调用构造函数，没有移动构造函数，也没有拷贝构造函数，比push_back的右值版本还快，可以代替push_back
//      Inserts a new element at the end of the vector, right after its current last element.
//      This new element is constructed in place using args as the arguments for its constructor.
        std::vector<int> myvector = {10, 20, 30};

        myvector.emplace_back(100);
        myvector.emplace_back(200);

        std::cout << "myvector contains:";
        for (auto& x: myvector)
        {
            std::cout << ' ' << x;
        }
        std::cout << '\n';
    }

    {
//    The container is extended by inserting a new element at position.
//    This new element is constructed in place using args as the arguments for its construction.
        std::vector<int> myvector = {10, 20, 30};

        auto it = myvector.emplace(myvector.begin() + 1,
                                   100); // 返回值：An iterator that points to the newly emplaced element.
        myvector.emplace(it, 200);
        myvector.emplace(myvector.end(), 300);

        std::cout << "myvector contains:";
        for (auto& x: myvector)
        {
            std::cout << ' ' << x;
        }
        std::cout << '\n';
    }
}

ADD_TEST(VectorTest, VectorEmplaceTest);

/*
 2、修整空间
 在一个应用中，可能会需要向一个vector中插入很多记录，比如说100000条，为了避免在插入过程中移动内存，咱实现向系统预订一段足够的连续的空间，例如
 vector<int> ivec;
 ivec.reserve(100000);
 这个问题是解决了。
 但是，如果后来这个vector不再需要存那么多的元素了，已经通过erase删除了。但是以前咱们预留的空间却无法被其他程序再度利用，这样会造成内存一定程度上的浪费。于是，我们利用目前的vector构造一个一模一样的vector，他并没有预留空间，于是以前预留的空间也被释放以作他用了：
 ivec.swap(vector<int>(ivec)); // or vector<int>(ivec).swap(ivec)
 或者如下所示 加一对大括号都可以，意思一样的：
 {
 std::vector<int> tmp = ivec;
 ivec.swap(tmp);
 }
 加一对大括号是可以让tmp退出{}的时候自动析构

 使用这种方法的前提是vector从前存储了大量数据，比如10000000，经过各种处理后，现在只有100条，那么向清空原来数据所占有的空间，就可以通过这种交换技术swap技法就是通过交换函数swap（），使得vector离开其自身的作用域，从而强制释放vector所占的内存空间。
 */

void ListTest(pid_t stPID, const char* pszProcessName)
{
    cout << "---list test---" << endl;

    cout << "construct" << endl;
    GetCurTime(g_tvStart);
    list<int> c(number_count_32M); // 虽然元素是int占4个字节，但是由于是双向链表，加上一个prev指针和一个next指针，一个结点要占12字节
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "size_" << endl;
    GetCurTime(g_tvStart);
    c.size(); // 当list比较大时，获取其size比较耗时，一般用定义一个变量自己管理list的size
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "advance" << endl;
    GetCurTime(g_tvStart);
    list<int>::iterator it1 = c.begin();
    advance(it1, number_count_32M / 2); // 非随机访问的容器，advance的效率都比较低。如果需要在外部遍历，一般先遍历容器把全部元素snapshot出来
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "Find" << endl;
    GetCurTime(g_tvStart);
    list<int>::iterator it2 = find(c.begin(), c.end(), 111111); // list也没有find成员
    if (it2 == c.end())
    {
        // 没找到
    }
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "erase" << endl;
    GetCurTime(g_tvStart);
    list<int>::iterator it3 = c.erase(it2); // 函数删除以loc指示位置的元素, 或者删除first和last之间的元素。 返回值是一个迭代器，指向最后一个被删除元素的下一个元素。
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "Insert" << endl;
    GetCurTime(g_tvStart);
    list<int>::iterator it4 = c.insert(it3, 222222); // 在loc之前插入新元素，返回值是一个迭代器，指向被插入的元素
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "swap" << endl;
    list<int> c2(number_count_32M);
    GetCurTime(g_tvStart);
    c.swap(c2); // clear会释放list的内存
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);
    c2.clear();

    cout << "clear" << endl;
    GetCurTime(g_tvStart);
    c.clear(); // clear会释放list的内存
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    NOT_USED(it4);
}

void MapTest(pid_t stPID, const char* pszProcessName)
{
    cout << "---map test---" << endl;

    cout << "construct" << endl;
    GetCurTime(g_tvStart);
    map<int, int> c; // map是一个关联数组，底层是红黑树结构
    for (unsigned int i = 0; i < number_count_16M; ++i)
    {
        if (!c.insert(map<int, int>::value_type(i, i)).second)
        {
            cout << "map Insert failed! number count: " << i << endl;
        }
    }
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "size_" << endl;
    GetCurTime(g_tvStart);
    c.size();
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "advance" << endl;
    GetCurTime(g_tvStart);
    map<int, int>::iterator it1 = c.begin();
    advance(it1, number_count_32M / 2);
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "Find" << endl;
    GetCurTime(g_tvStart);
    map<int, int>::iterator it2 = c.find(111111);
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "erase" << endl;
    GetCurTime(g_tvStart);
    int iItemCount = c.erase(222222); // 函数删除在loc位置的元素，或者删除在first和last之间的元素，或者删除那些值为x所有元素。
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "Insert" << endl;
    GetCurTime(g_tvStart);
    bool bRet = c.insert(map<int, int>::value_type(222222, 222222)).second;
    if (!bRet)
    {
        // 插入失败，可能是因为key重复了
    }
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "swap" << endl;
    map<int, int> c2;
    for (unsigned int i = 0; i < number_count_16M; ++i)
    {
        if (!c2.insert(map<int, int>::value_type(i, i)).second)
        {
            cout << "map Insert failed! number count: " << i << endl;
        }
    }
    GetCurTime(g_tvStart);
    c.swap(c2);
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);
    c2.clear();

    cout << "clear" << endl;
    GetCurTime(g_tvStart);
    c.clear(); // clear会释放map的内存
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    NOT_USED(it2);
    NOT_USED(iItemCount);
}

void HMapTest(pid_t stPID, const char* pszProcessName)
{
    cout << "---hash map test---" << endl;

    cout << "construct" << endl;
    GetCurTime(g_tvStart);
    __hash_map<int, int> c;
    for (unsigned int i = 0; i < number_count_16M; ++i)
    {
        if (!c.insert(map<int, int>::value_type(i, i)).second)
        {
            cout << "map Insert failed! number count: " << i << endl;
        }
    }
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "size_" << endl;
    GetCurTime(g_tvStart);
    c.size();
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "advance" << endl;
    GetCurTime(g_tvStart);
    __hash_map<int, int>::iterator it1 = c.begin();
    advance(it1, number_count_32M / 2);
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "Find" << endl;
    GetCurTime(g_tvStart);
    __hash_map<int, int>::iterator it2 = c.find(111111);
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "erase" << endl;
    GetCurTime(g_tvStart);
    c.erase(222222); // 返回值？
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "Insert" << endl;
    GetCurTime(g_tvStart);
    bool bRet = c.insert(__hash_map<int, int>::value_type(222222, 222222)).second;
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "swap" << endl;
    __hash_map<int, int> c2;
    for (unsigned int i = 0; i < number_count_16M; ++i)
    {
        if (!c2.insert(map<int, int>::value_type(i, i)).second)
        {
            cout << "map Insert failed! number count: " << i << endl;
        }
    }
    GetCurTime(g_tvStart);
    c.swap(c2);
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);
    c2.clear();

    cout << "clear" << endl;
    GetCurTime(g_tvStart);
    c.clear(); // clear会释放hash map的内存
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    NOT_USED(it2);
    NOT_USED(bRet);
}

void SetTest(pid_t stPID, const char* pszProcessName)
{
    cout << "---set test---" << endl;

    cout << "construct" << endl;
    GetCurTime(g_tvStart);
    set<int> c; // set底层也是红黑树结构
    for (unsigned int i = 0; i < number_count_16M; ++i)
    {
        if (!c.insert(i).second)
        {
            cout << "set Insert failed! number count: " << i << endl;
        }
    }
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "size_" << endl;
    GetCurTime(g_tvStart);
    c.size();
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "advance" << endl;
    GetCurTime(g_tvStart);
    set<int>::iterator it1 = c.begin();
    advance(it1, number_count_32M / 2);
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "Find" << endl;
    GetCurTime(g_tvStart);
    set<int>::iterator it2 = c.find(111111);
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "erase" << endl;
    GetCurTime(g_tvStart);
    int iItemCount = c.erase(222222); // 删除set中position处元素（无返回值）；删除等于x值的所有元素（返回被删除的元素的个数）；删除从first开始到last结束的元素（无返回值）。
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "Insert" << endl;
    GetCurTime(g_tvStart);
    bool bRet = c.insert(222222).second; // 在当前集合中插入x元素，并返回指向该元素的迭代器和一个布尔值来说明x是否成功的被插入了。
    if (!bRet)
    {
        // 插入失败，可能是key重复了
    }
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "swap" << endl;
    set<int> c2;
    for (unsigned int i = 0; i < number_count_16M; ++i)
    {
        if (!c2.insert(i).second)
        {
            cout << "set Insert failed! number count: " << i << endl;
        }
    }
    GetCurTime(g_tvStart);
    c.swap(c2);
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);
    c2.clear();

    cout << "clear" << endl;
    GetCurTime(g_tvStart);
    c.clear(); // clear会释放set的内存
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    NOT_USED(it2);
    NOT_USED(iItemCount);
}

void DequeTest(pid_t stPID, const char* pszProcessName)
{
    /*
     顾名思义，deque是一个“双端队列（double-ended queue）”，这意味着从队列开始和结束处插入（删除）数据的性能很好。为了达到这个目的，std::deque基于一种分段连续的、介于数组和链表之间的数据结构，示意如下：
     template<class _E>
     class deque
     {
     enum { BlockSize = 512 };
     typedef _E Block[BlockSize]; // Block是一个“含有512和_E类型的元素的数组“类型
     std::vector<Block*> m_storage; // vector的元素是指向数组的指针
     iterator m_first, m_last;
     };
     其中，iterator是deque::iterator类，m_first和m_last分别就是容器的begin()和end()。之所以需要它们，是因为m_storage的第一个Block和最后一个Block的数据可能没有填满，需要有指针去指出边界。设想一下如果 我们只是要实现一个单向的队列，那么可以去掉这里的m_first成员（因为第一个Block如果它不同时是最后一个Block，则不会不满）。
     为什么需要采用这种分段连续的数据结构呢？答案是：性能。deque 平常很少为C++程序员所使用，但从容器的各方面的性能指标来看，实在不应该如此。可以说，deque 是 STL 中基于值的容器（它们包括：list/slist， vector， deque， basic_string等）中综合性能最优的类。下面我们仔细分析一下。
     时间性能分析
     push_back/push_front
     这两个操作对 deque 来说并无区别。而 vector 则不支持 push_front（因为性能很差而不提供）。我们对比各容器 push_back 性能。如下：
     vector
     vector：：push_back 的性能具体要看 vector 的实现，主要关乎vector的内存增长模型。目前所见典型的做法是N*2模型，也就是说在申请的内存填满后，申请N*2字节的内存，这里N为当前 vector占用的空间。在此情况下，元素搬移的时间为 （1/N * N） = 1 为常数（其中1/N为平均搬移的次数，N为每次搬移的数据量），故此 push_back 的复杂度为 O（1）。但这种做法时间性能是有了，空间存在巨大的浪费。但是如果增长模型为N+Delta模型（这里Delta为一个常增长系数），那么元素搬移的时 间为 （1/Delta * N） = O（N）。空间是节约了，但时间性能极差。Erlang语言引入了一个很有意思的增长模型，是基于 Fibonacci 序列，空间浪费要好于N*2模型，兼顾了空间性能和时间性能。
     list
     list：：push_back 的性能为O（1）。主要的时间开销为new Node时间。如果我们使用GC Allocator，list：：push_back的速度非常快。
     deque
     deque：：push_back 的性能接近O（1）。之所以不是O（1），是因为 m_storage 满了之后，会导致和 vector 一样的内存搬移问题。假设 vector<Block*> 采用了 2*N 增长模型，那么 deque：：push_back 性能显然是 O（1）。如果采用 N+Delta 模型，那么元素搬移时间为 （1/（BlockSize*Delta） * N/BlockSize） = O（N）。虽然也是 O（N），但是一个是 N/Delta，一个是 N/（Delta*BlockSize*BlockSize），还是差别很大。由于 m_storage.size_（） 通常很小，所以实际情况下哪怕在海量数据情形下 deque：：push_back 仍然表现良好。

     operator[]
     operator[] 是指通过下标取数据。显然 list 的复杂度为O（N），非常慢。而 vector、deque 均为 O（1）。让我们想象下 deque：：operator[] 的实现：
     _E　deque::operator[](int　i)
     {
     return　m_storage[i/BlockSize][i%BlockSize];
     }
     可以看出，deque 只比 vector 多了一次内存访问。

     空间性能分析
     push_back
     vector
     很不幸，如果 vector 采用 N*2 的内存增长模型（通常如此），那么在最差的情况下，空间复杂度就是 2*N ，最好的情况下为 N（所有的内存都用上了）。平均来讲，空间复杂度为 1.5*N .也就是说，通常差不多有一半的内存是被浪费的。

     list
     list 的空间浪费与 vector 相比不遑多让。它的空间复杂度为 （1 + sizeof（pointer）*2/sizeof（_E））*N.如果我们让 list 存储的元素为 pointer（即 _E = pointer），那么空间复杂度为 3*N，比 vector 还浪费。

     deque
     deque 的最差情况下的空间复杂度为 N + sizeof（pointer）*2*N/（BlockSize*sizeof（_E））（这里假设vector<Block*>也采用 2*N 增长模型，平均复杂度则将式中2改为1.5即可）。如果我们保存的元素为 pointer（即 _E = pointer），并且BlockSize取512，那么空间复杂度为 N + N/256.也就是说最差情况下只浪费了 N/256 的内存。

     deque的其他特性
     元素地址不变
     由于 deque 并不进行数据搬移，带来一个有意思的特性，就是 deque 的元素地址在只有 push_back/push_front，没有 Insert/erase 时，可保持元素地址不变。
     需要注意的是，vector 并不具备这样的特性。如下的代码是不合法的：
     std::vector<int>　vec;
     ...
     int&　elem　=　vec[i];
     vec.push_back(100);
     elem　=　99;　//　error:　can't　access　elem　since　vec　was　changed!
     由于取得 elem 之后存在 push_back 操作，所获得的元素地址（&elem）可能会由于内存搬移而失效。但是如果我们将容器换为 std：：deque<int>，则这个代码不会有任何问题。
     std::deque<int>　dq;
     ...
     int&　elem　=　dq[i];
     dq.push_back(100);
     elem　=　99;　//　ok!
     另外需要注意的是，元素地址不变，并不代表 iterator 不变，如下的代码 deque 并不支持：
     std::deque<int>　dq;
     ...
     std::deque<int>::iterator　it　=　dq.begin()　+　i;
     dq.push_back(100);
     *it　=　99;　//　error:　can't　access　iterator　since　deque　was　changed!

     结论：通过 vector， list， deque 的时间、空间性能对比，我们可以看出，应该提倡尽可能使用 deque 这个容器。特别是，如果要承受海量数据，deque 是最合适的人选了。
     */

    cout << "---deque test---" << endl;

    cout << "construct" << endl;
    GetCurTime(g_tvStart);
    deque<int> c(number_count_128M); // deque中一个结点的结构？
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "size_" << endl;
    GetCurTime(g_tvStart);
    c.size();
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "advance" << endl;
    GetCurTime(g_tvStart);
    deque<int>::iterator it1 = c.begin();
    advance(it1, number_count_32M / 2);
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "Find" << endl;
    GetCurTime(g_tvStart);
    deque<int>::iterator it2 = find(c.begin(), c.end(), 111111); // deque没有find函数
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "erase" << endl;
    GetCurTime(g_tvStart);
    deque<int>::iterator it3 = c.erase(it2); // 删除positon位置上的元素，或者删除first和last之间的所有元素。返回值是一一个迭代器指向被删除元素的后一个元素。
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "Insert" << endl;
    GetCurTime(g_tvStart);
    deque<int>::iterator it4 = c.insert(it3,
                                        222222); // 在positon前插入n个x值，或者插入从first到last范围内的元素到postion前面。只有在插入一个元素时，才有返回值，返回一个迭代器 ，指向新插入的元素。
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "swap" << endl;
    deque<int> c2(number_count_128M);
    GetCurTime(g_tvStart);
    c.swap(c2);
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);
    c2.clear();

    cout << "clear" << endl;
    GetCurTime(g_tvStart);
    c.clear(); // clear会释放deque的内存
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    NOT_USED(it4);
}

void QueueTest(pid_t stPID, const char* pszProcessName)
{
    // 队列：FIFO，在队尾插入，在队首删除
    cout << "---queue test---" << endl;

}

void PriorityQueueTest(pid_t stPID, const char* pszProcessName)
{
    /*
     PriorityQueue是个基于优先级堆的极大优先级队列。
     priority_queue调用 STL里面的 make_heap(), pop_heap(), push_heap() 算法实现，也算是堆的另外一种形式。堆是什么？二叉树、完全二叉树？
     PriorityQueue对元素采用的是堆排序，头是按指定排序方式的最小元素。堆排序只能保证根是最大（最小），整个堆并不是有序的。

     priority_queue 对于基本类型的使用方法相对简单。他的模板声明带有三个参数:
     priority_queue<Type, Container, Functional>
     其中Type 为数据类型， Container 为保存数据的容器，Functional 为元素比较方式。
     Container 必须是用数组实现的容器，比如 vector, deque 但不能用 list.
     STL里面默认用的是 vector. 比较方式默认用 operator< , 所以如果你把后面俩个参数缺省的话，
     优先队列就是大顶堆，队头元素最大（如果有多个元素的值相等，则任意取一个。


     如果要用到小顶堆，则一般要把模板的三个参数都带进去。
     STL里面定义了一个仿函数 greater<>，对于基本类型可以用这个仿函数声明小顶堆，如下：
     priority_queue<int, vector<int>, greater<int> > q;

     对于自定义类型，则必须自己重载 operator< 或者自己写仿函数
     struct Node{
     int x, y;
     Node( int a= 0, int b= 0 ):
     x(a), y(b) {}
     };

     bool operator<( Node a, Node b ){
     if( a.x== b.x ) return a.y> b.y;
     return a.x> b.x;
     }

     priority_queue<Node> q;


     或者：
     struct Node{
     int x, y;
     Node( int a= 0, int b= 0 ):
     x(a), y(b) {}
     };

     struct cmp{
     bool operator() ( Node a, Node b ){
     if( a.x== b.x ) return a.y> b.y;

     return a.x> b.x; }
     };

     priority_queue<Node, vector<Node>, cmp> q;
     */
    cout << "---priority queue test---" << endl;

}

void StackTest(pid_t stPID, const char* pszProcessName)
{
    // 栈：LIFO，后进先出
    cout << "---stack test---" << endl;

}

void BitsetTest(pid_t stPID, const char* pszProcessName)
{
    cout << "---bitset test---" << endl;

}

void MultisetTest(pid_t stPID, const char* pszProcessName)
{
    cout << "---multiset test---" << endl;

    cout << "construct" << endl;
    GetCurTime(g_tvStart);
    multiset<int> c; // multiset中一个结点的结构？
    for (unsigned int i = 0; i < number_count_16M; ++i)
    {
        c.insert(i);
    }
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "size_" << endl;
    GetCurTime(g_tvStart);
    c.size();
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "advance" << endl;
    GetCurTime(g_tvStart);
    multiset<int>::iterator it1 = c.begin();
    advance(it1, number_count_32M / 2);
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "Find" << endl;
    GetCurTime(g_tvStart);
    multiset<int>::iterator it2 = c.find(111111);
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "erase" << endl;
    GetCurTime(g_tvStart);
    int iItemCount = c.erase(222222); // 删除set中position处元素（无返回值）；删除等于x值的所有元素（返回被删除的元素的个数）；删除从first开始到last结束的元素（无返回值）。
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "Insert" << endl;
    GetCurTime(g_tvStart);
    c.insert(222222); // 在当前集合中插入x元素，并返回指向该元素的迭代器和一个布尔值来说明x是否成功的被插入了。
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "swap" << endl;
    multiset<int> c2;
    for (unsigned int i = 0; i < number_count_16M; ++i)
    {
        c2.insert(i);
    }
    GetCurTime(g_tvStart);
    c.swap(c2);
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "clear" << endl;
    GetCurTime(g_tvStart);
    c.clear(); // clear会释放set的内存
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    NOT_USED(it2);
    NOT_USED(iItemCount);
}

void MultimapTest(pid_t stPID, const char* pszProcessName)
{
    cout << "---multimap test---" << endl;

    cout << "construct" << endl;
    GetCurTime(g_tvStart);
    multimap<int, int> c; // multimap中一个结点的结构？
    for (unsigned int i = 0; i < number_count_16M; ++i)
    {
        c.insert(multimap<int, int>::value_type(i, i));
    }
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "size_" << endl;
    GetCurTime(g_tvStart);
    c.size();
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "advance" << endl;
    GetCurTime(g_tvStart);
    multimap<int, int>::iterator it1 = c.begin();
    advance(it1, number_count_32M / 2);
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "Find" << endl;
    GetCurTime(g_tvStart);
    multimap<int, int>::iterator it2 = c.find(111111);
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "erase" << endl;
    GetCurTime(g_tvStart);
    int iItemCount = c.erase(222222); // 函数删除在loc位置的元素，或者删除在first和last之间的元素，或者删除那些值为x所有元素。
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "Insert" << endl;
    GetCurTime(g_tvStart);
    c.insert(multimap<int, int>::value_type(222222, 222222));
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    cout << "swap" << endl;
    multimap<int, int> c2;
    for (unsigned int i = 0; i < number_count_16M; ++i)
    {
        c2.insert(multimap<int, int>::value_type(i, i));
    }
    GetCurTime(g_tvStart);
    c.swap(c2);
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    c2.clear();

    PrintMemUsed(stPID, pszProcessName);
    cout << "clear" << endl;
    GetCurTime(g_tvStart);
    c.clear(); // clear会释放map的内存
    GetCurTime(g_tvEnd);
    PrintTimeDiff(g_tvStart, g_tvEnd);
    PrintMemUsed(stPID, pszProcessName);

    NOT_USED(it2);
    NOT_USED(iItemCount);
}

void STLTest(const char* pszProcessName)
{
    pid_t stPID = getpid();
    //const char* pszProcessName = argv[0];
    cout << "pid: " << stPID << ", process name: " << pszProcessName << endl;

    VectorTest(stPID, pszProcessName);
    ListTest(stPID, pszProcessName);
    MapTest(stPID, pszProcessName);
    HMapTest(stPID, pszProcessName);
    SetTest(stPID, pszProcessName);
    DequeTest(stPID, pszProcessName);
    QueueTest(stPID, pszProcessName);
    PriorityQueueTest(stPID, pszProcessName);
    StackTest(stPID, pszProcessName);
    BitsetTest(stPID, pszProcessName);
    MultisetTest(stPID, pszProcessName);
    MultimapTest(stPID, pszProcessName);
}

// trie树，适合字符串查找

// 编译：g++ stl-test.cpp -g -Wall
// 运行时需要内存大一点的机器，否则会因为分配内存失败而宕机

/*
 运行结果：

 */

/*
 自定义结构放入map、set、hash_map中
 自定义结构放入vector中按结构中的数据降序排序
 自定义结构的指针放入vector中按结构中的数据降序排序
 */
}

/**
 * C++中，当vector中的数据类型为基本类型时，调用std::sort函数很容易实现 vector中数据成员的升序或降序排列，
 * 但是当vector中的数据类型为自定义结构体时，该怎样实现升序与降序排列呢？
 */

using namespace std;

namespace vector_sort_test
{
struct TItem
{
    int _type;
    int _ID;

    bool operator<(const TItem& rhs) const // 升序排序时必须重载的运算符
    {
        return _type < rhs._type;
    }

    bool operator>(const TItem& rhs) const // 降序排序时必须重载的运算符
    {
        return _type > rhs._type;
    }
};

// 方法1：重载运算符
void Test001()
{
    vector<TItem> itemVec;

    TItem item1;
    item1._type = 1;
    item1._ID = 1;

    TItem item2;
    item2._type = 2;
    item2._ID = 2;

    TItem item3;
    item3._type = 3;
    item3._ID = 3;

    TItem item4;
    item4._type = 2;
    item4._ID = 4;

    itemVec.push_back(item1);
    itemVec.push_back(item2);
    itemVec.push_back(item3);
    itemVec.push_back(item4);

    // 升序排序
    sort(itemVec.begin(), itemVec.end(), less<TItem>());
    // 或者sort(itemVec.begin(), itemVec.end()); 默认情况为升序排列

    for (size_t i = 0; i < itemVec.size(); ++i)
    {
        printf("type: %d, id: %d\n", itemVec[i]._type, itemVec[i]._ID);
    }

    printf("--\n");

    // 降序排序
    sort(itemVec.begin(), itemVec.end(), greater<TItem>());

    for (size_t i = 0; i < itemVec.size(); ++i)
    {
        printf("type: %d, id: %d\n", itemVec[i]._type, itemVec[i]._ID);
    }
}

// 方法2： 全局的比较函数（因为vector中存放的是TItem对象，不是其指针，所以比较函数的参数为引用类型的）
bool LessMark(const TItem& item1, const TItem& item2)
{
    return item1._type < item2._type;
}

bool GreaterMark(const TItem& item1, const TItem& item2)
{
    return item1._type > item2._type;
}

void Test002()
{
    vector<TItem> itemVec;

    TItem item1;
    item1._type = 1;
    item1._ID = 1;

    TItem item2;
    item2._type = 2;
    item2._ID = 2;

    TItem item3;
    item3._type = 3;
    item3._ID = 3;

    TItem item4;
    item4._type = 2;
    item4._ID = 4;

    itemVec.push_back(item1);
    itemVec.push_back(item2);
    itemVec.push_back(item3);
    itemVec.push_back(item4);

    sort(itemVec.begin(), itemVec.end(), LessMark); // 升序排序

    for (size_t i = 0; i < itemVec.size(); i++)
    {
        printf("type: %d, id: %d\n", itemVec[i]._type, itemVec[i]._ID);
    }

    printf("--\n");

    sort(itemVec.begin(), itemVec.end(), GreaterMark); // 降序排序

    for (size_t i = 0; i < itemVec.size(); i++)
    {
        printf("type: %d, id: %d\n", itemVec[i]._type, itemVec[i]._ID);
    }
}

// 当vector中存放的是TItem的指针时，全局比较函数的参数要求是指针类型的。
bool LessMarkPtr(const TItem* item1, const TItem* item2)
{
    return item1->_type < item2->_type;
}

bool GreaterMarkPtr(const TItem* item1, const TItem* item2)
{
    return item1->_type > item2->_type;
}

void Test003()
{
    vector<TItem*> itemVec;

    TItem item1;
    item1._type = 1;
    item1._ID = 1;

    TItem item2;
    item2._type = 2;
    item2._ID = 2;

    TItem item3;
    item3._type = 3;
    item3._ID = 3;

    TItem item4;
    item4._type = 2;
    item4._ID = 4;

    itemVec.push_back(&item1);
    itemVec.push_back(&item2);
    itemVec.push_back(&item3);
    itemVec.push_back(&item4);

    sort(itemVec.begin(), itemVec.end(), LessMarkPtr); // 升序排序

    for (size_t i = 0; i < itemVec.size(); i++)
    {
        printf("type: %d, id: %d\n", itemVec[i]->_type, itemVec[i]->_ID);
    }

    printf("--\n");

    sort(itemVec.begin(), itemVec.end(), GreaterMarkPtr); // 降序排序

    for (size_t i = 0; i < itemVec.size(); i++)
    {
        printf("type: %d, id: %d\n", itemVec[i]->_type, itemVec[i]->_ID);
    }
}

// 方法3： 函数对象（因为vector中存放的是TItem对象，不是其指针，所以比较函数的参数为引用类型的）
class CompareLess
{
public:
    bool operator()(const TItem& item1, const TItem& item2)
    {
        return item1._type < item2._type;
    }
};

class CompareGreater
{
public:
    bool operator()(const TItem& item1, const TItem& item2)
    {
        return item1._type > item2._type;
    }
};

void Test004()
{
    vector<TItem> itemVec;

    TItem item1;
    item1._type = 1;
    item1._ID = 1;

    TItem item2;
    item2._type = 2;
    item2._ID = 2;

    TItem item3;
    item3._type = 3;
    item3._ID = 3;

    TItem item4;
    item4._type = 2;
    item4._ID = 4;

    itemVec.push_back(item1);
    itemVec.push_back(item2);
    itemVec.push_back(item3);
    itemVec.push_back(item4);

    sort(itemVec.begin(), itemVec.end(), CompareLess()); // 升序排序

    for (size_t i = 0; i < itemVec.size(); i++)
    {
        printf("type: %d, id: %d\n", itemVec[i]._type, itemVec[i]._ID);
    }

    printf("--\n");

    sort(itemVec.begin(), itemVec.end(), CompareGreater()); // 降序排序

    for (size_t i = 0; i < itemVec.size(); i++)
    {
        printf("type: %d, id: %d\n", itemVec[i]._type, itemVec[i]._ID);
    }
}

// 当vector中存放的是TItem的指针时，函数对象的比较函数的参数要求是指针类型的。
class CompareLessPtr
{
public:
    bool operator()(const TItem* item1, const TItem* item2)
    {
        return item1->_type < item2->_type;
    }
};

class CompareGreaterPtr
{
public:
    bool operator()(const TItem* item1, const TItem* item2)
    {
        return item1->_type > item2->_type;
    }
};

void Test005()
{
    vector<TItem*> itemVec;

    TItem item1;
    item1._type = 1;
    item1._ID = 1;

    TItem item2;
    item2._type = 2;
    item2._ID = 2;

    TItem item3;
    item3._type = 3;
    item3._ID = 3;

    TItem item4;
    item4._type = 2;
    item4._ID = 4;

    itemVec.push_back(&item1);
    itemVec.push_back(&item2);
    itemVec.push_back(&item3);
    itemVec.push_back(&item4);

    sort(itemVec.begin(), itemVec.end(), CompareLessPtr()); // 升序排序

    for (size_t i = 0; i < itemVec.size(); i++)
    {
        printf("type: %d, id: %d\n", itemVec[i]->_type, itemVec[i]->_ID);
    }

    printf("--\n");

    sort(itemVec.begin(), itemVec.end(), CompareGreaterPtr()); // 降序排序

    for (size_t i = 0; i < itemVec.size(); i++)
    {
        printf("type: %d, id: %d\n", itemVec[i]->_type, itemVec[i]->_ID);
    }
}

// 推荐使用函数对象。
ADD_TEST(VectorSortTest, Test001);
ADD_TEST(VectorSortTest, Test002);
ADD_TEST(VectorSortTest, Test003);
ADD_TEST(VectorSortTest, Test004);
ADD_TEST(VectorSortTest, Test005);
} /* namespace vector_sort_test */

namespace multimap_test
{
/**
 * multimap 操作lower_bound、 upper_bound、 equal_range
 */

using namespace std;

static void Test001()
{
    //定义 multimap 对象 author
    multimap<string, string> author; //第一个string 表示作者，第二个string 表示作者所著书名

    //添加作者author1 写的3本书 book11~book13 ,3种添加方式
    author.insert(
        multimap<string, string>::value_type(string("author1"),
                                             string("book11")));
    author.insert(multimap<string, string>::value_type("author1", "book12"));
    author.insert(make_pair(string("author1"), string("book13")));

    //添加作者author2 写的2本书 book21~book22
    author.insert(multimap<string, string>::value_type("author2", "book21"));
    author.insert(make_pair(string("author2"), string("book22")));

    //给出author 的键author1，查找此键关联的所有值
    string search_item("author1");

    //multimap_size 表示 作者著书 数量
    multimap<string, string>::size_type multimap_size;

    //返回 search_item 的记录数，如果不存在返回0
    multimap_size = author.count(search_item);
    cout << "multimap_size : " << multimap_size << "\n";

    //定义迭代器 it ,
    multimap<string, string>::iterator it;

    //用 Find 查找
    it = author.find(search_item); //如果存在指针指向键关联的第一个值
    multimap<string, string>::size_type st;
    //it、multimap_size 配合使用，操作键关联的所有值
    for (st = 0; st != multimap_size; st++, it++)
    {
        cout << it->first << "  " << it->second << "\n";
    }

    //用 lower_bound,upper_bound 操作键关联的值
    cout << "use lower_bound and upper_bound function :\n";
    //如果 author 中存在键search_item ,begin指向第一个匹配的元素
    multimap<string, string>::iterator begin = author.lower_bound(search_item);
    //如果 author 中存在键search_item ,end指向最后一个匹配的元素的下一位置
    multimap<string, string>::iterator end = author.upper_bound(search_item);
    while (begin != end)
    {
        cout << begin->first << " " << begin->second << "\n";
        begin++;
    }

    //用 equal_range 操作键关联的值
    cout << "use equal_range function : \n";
    //定义pair 对象position; pair数据类型是 2个 multimap<string,string>::iterator 指针。
    pair<multimap<string, string>::iterator, multimap<string, string>::iterator> position;
    //如果键存在，函数返回2个指针，第一个指针指向键第一个匹配的元素
    //第二个指针指向键最后一个匹配的元素的下一位置
    position = author.equal_range(search_item);
    while (position.first != position.second)
    {
        cout << position.first->first << "  " << position.first->second << "\n";
        position.first++;
    }
    cout << endl;
}

ADD_TEST(MultiMapTest, Test001);

} // multimap_test