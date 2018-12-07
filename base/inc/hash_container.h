/**
 * @file hash_container.h
 * @brief
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_HASH_CONTAINER_H_
#define BASE_INC_HASH_CONTAINER_H_

// http://fgda.pl/post/7/gcc-hash-map-vs-unordered-map

#if (defined(__linux__))
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

/**
 * @attention DO NOT USE hash<const char*> EVER AGAIN.
 * operator[] is supposed to insert the key into the map,
 * copying it invoking the default constructor (see the reference),
 * in this case, it will simply copy buffer[0].
 */

// map与multimap相比：
// 两者底层实现均为红黑树，但是multimap支持重复的键，不支持[]运算符
//
// map与unordered_map相比：
// map底层实现为红黑树，undered_map底层实现为哈希表，两者均不能有重复的建，均支持[]运算符
//
// unordered_map与unordered_multimap相比：
// 两者实现均为哈希表，但是unordered_multimap支持重复的键，不支持[]运算符

// multimap与unordered_multimap相比：
// multimap底层实现为红黑树，undered_multimap底层实现为哈希表，两者均支持重复的键，均不支持[]运算符

#if (defined(__linux__))
#ifdef __hash_map
#undef __hash_map
#endif

#ifdef __hash_multimap
#undef __hash_multimap
#endif

#ifdef __hash_set
#undef __hash_set
#endif

#ifdef __hash_multiset
#undef __hash_multiset
#endif

#if GCC_VERSION >= 40300
#include <unordered_map>
#include <unordered_set>
// DO NOT USE hash<const char*> EVER AGAIN
#define __hash_map std::unordered_map
#define __hash_multimap std::unordered_multimap
#define __hash_set std::unordered_set
#define __hash_multiset std::unordered_multiset
#else
#include <ext/__hash_map>
#define __hash_map __gnu_cxx::__hash_map
#define __hash_multimap __gnu_cxx::__hash_multimap
#define __hash_set __gnu_cxx::__hash_set
#define __hash_multiset __gnu_cxx::__hash_multiset
#endif
#endif
#endif // BASE_INC_HASH_CONTAINER_H_
