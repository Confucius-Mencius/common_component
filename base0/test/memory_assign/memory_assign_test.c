/**
 编译：
 gcc -o memory_assign memory_assign.c

 把可执行程序memory_assign的信息导出到文本文件1.txt中：
 readelf -a memory_assign >1.txt

 查看1.txt，可以看到.text, .data, .bss各段的起始地址和长度，
 或者使用objdump -h查看段头：objdump -h memory_assign >2.txt

 执行程序：
 memory_assign >3.txt

 对照1.txt和3.txt观察
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int g_const_int = 10; // .rodata
const char g_const_str[] = "123456"; // .rodata

int g_int_init = 20; // .data
int g_int_uninit; // .bss

char* g_pointer_init = "123456"; // 指针指向的地址在.rodata段，指针变量自身在.data段
char* g_pointer_uninit; // 指针指向的地址为nil，指针变量自身在.bss段

static int g_static_int_init = 30; // .data
static int g_static_int_uninit; // .bss

int main()
{
    const int local_const_int = 10; // 栈
    const char local_const_str[] = "123456"; // 栈

    int local_int_init = 20; // 栈
    int local_int_uninit; // 栈

    char* local_pointer_init = "123456"; // 指针指向的地址在.rodata段，指针变量自身在栈中
    char* local_pointer_uninit; // 指针指向的地址未初始化（为oxcccccccc），不可访问，指针变量自身在栈中

    static int local_static_int_init = 30; // .data
    static int local_static_int_uninit; // .bss

    char* pointer_to_heap = (char*) malloc(10); // 指针指向的地址在对中，指针变量自身在栈中
    strcpy(pointer_to_heap, "123456");

    printf("[const int g_const_int = 10;]                 addr: %p\n",
           &g_const_int);
    printf("[const char g_const_str[] = \"123456\";]      addr: %p\n",
           &g_const_str);
    printf("[int g_int_init = 20;]                        addr: %p\n",
           &g_int_init);
    printf("[int g_int_uninit;]                           addr: %p\n",
           &g_int_uninit);
    printf("[char* g_pointer_init = \"123456\";]          addr: %p\n",
           g_pointer_init);
    printf("[char* g_pointer_uninit;]                     addr: %p\n",
           g_pointer_uninit);
    printf("&[char* g_pointer_init = \"123456\";]         addr: %p\n",
           &g_pointer_init);
    printf("&[char* g_pointer_uninit;]                    addr: %p\n",
           &g_pointer_uninit);
    printf("[static int g_static_int_init = 30;]          addr: %p\n",
           &g_static_int_init);
    printf("[static int g_static_int_uninit;]             addr: %p\n",
           &g_static_int_uninit);

    printf("[const int local_const_int = 10;]             addr: %p\n",
           &local_const_int);
    printf("[const char local_const_str[] = \"123456\";]  addr: %p\n",
           &local_const_str);
    printf("[int local_int_init = 20;]                    addr: %p\n",
           &local_int_init);
    printf("[int local_int_uninit;]                       addr: %p\n",
           &local_int_uninit);
    printf("[char* local_pointer_init = \"123456\";]      addr: %p\n",
           local_pointer_init);
    //printf("[char* local_pointer_uninit;]               addr: %p\n", local_pointer_uninit); 非法！指针未初始化，不可访问
    printf("&[char* local_pointer_init = \"123456\";]     addr: %p\n",
           &local_pointer_init);
    printf("&[char* local_pointer_uninit;]                addr: %p\n",
           &local_pointer_uninit);
    printf("[static int local_static_int_init = 30;]      addr: %p\n",
           &local_static_int_init);
    printf("[static int local_static_int_uninit;]         addr: %p\n",
           &local_static_int_uninit);

    printf("[char* pointer_to_heap = (char*) malloc(10);] addr: %p\n",
           pointer_to_heap);
    printf("&[char* pointer_to_heap = (char*) malloc(10);]addr: %p\n",
           &pointer_to_heap);

    return 0;
}
