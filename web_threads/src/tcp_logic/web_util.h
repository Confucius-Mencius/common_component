#ifndef WEB_THREADS_SRC_TCP_LOGIC_WEB_UTIL_H_
#define WEB_THREADS_SRC_TCP_LOGIC_WEB_UTIL_H_

#include <stddef.h>

//RFC3986文档规定，URL中只允许包含以下四种：
//1、英文字母（a-zA-Z）
//2、数字（0-9）
//3、-_.~ 4个特殊字符
//4、所有保留字符，RFC3986中指定了以下字符为保留字符（英文字符）： ! * ' ( ) ; : @ & = + $ , / ? # [ ]
//其他字符均为特殊字符，如果需要在URL中用到特殊字符，需要将这些特殊字符换成相应的十六进制的值，即做url encode
//但是由于历史原因，目前尚存在一些不标准的编码实现。例如虽然RFC3986文档规定，对于波浪符号~不需要进行URL编码，但是还是有很多老的网关或者传输代理会。

//这里的URL编码做了如下操作：
//字符"a"-"z"，"A"-"Z"，"0"-"9"，"."，"-"，"*"，和"_"等都不被编码，维持原值；
//空格" "被转换为加号"+"。
//其他每个字节都被表示成"%xy"格式的由3个字符组成的字符串，编码为UTF-8。就是每个字节(十六进制）前加一个'%'

char* url_encode(int& encoded_len, const char* s, size_t len); // 返回的字符串要调用url_free释放
size_t url_decode(char* s, size_t len);
void url_free(char* s); // 释放url_encode返回的字符串

#endif // WEB_THREADS_SRC_TCP_LOGIC_WEB_UTIL_H_
