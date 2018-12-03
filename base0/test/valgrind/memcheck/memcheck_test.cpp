#include <iostream>
#include <unistd.h>
#include <string.h>

// 内存泄露报告

int main(int argc, char* argv[])
{
    const int N = 10; // # of elements in array_
    const int LARGE_RANGE = 50; // 500 * 1024 * 1024;

    std::cout << "Start of tests" << std::endl;

    int* p1 = new int(1); // use to cause leak
    int* p2 = new int[N]; // allocate an int array_
    int* p3 = new int(2); // used to test wrong delete

    char* cp = NULL; // cp is null pointer
    char ca[3]; // unintialized array_
    char* pLarge = NULL; // used to test set address range perms: large range

    std::cout << "Func 1: off by one" << std::endl;
    for (int i = 1; i < N + 1; i++) // one-off in loop
    {
        p2[i] = i; // err - initialize element p[N]
    }

    std::cout << "Func 2: access freed storage" << std::endl;
    delete p1;
    *p1 = 3; // err - accessing freed storage

    std::cout << "Func 3: using uninitialized storage" << std::endl;
    if (p2[0])
    {
        std::cout << "Junk" << std::endl; // err - used uninit data
    }

    std::cout << "Func 4: delete array_ using scalar delete" << std::endl;
    delete p2; // err - delete array_ with scalar delete

    std::cout << "Func 5: array_ delete of scalar" << std::endl;
    delete[] p3; // err - array_ delete of scalar

    std::cout << "Func 6: overlapping storage blocks" << std::endl; // 可能不会报错
    memcpy(ca, &ca[1], 2); // err - overlapping storage blocks

    std::cout << "Func 7: system call using uninitialize data" << std::endl;
    sleep(1 & ca[0]); // err - uninit data in system call

    // 做试验时发现，如果LARGE_RANGE太大，例如500 * 1024 * 1024，会直接宕掉；
    // 如果LARGE_RANGE比较小，例如50，一切顺利。如果LARGE_RANGE为500，则分配正常，但delete[]时会宕掉。
    std::cout << "Func 8: set address range perms: large range" << std::endl; // 64位上给出的是warning
    pLarge = new char[LARGE_RANGE];
    delete[] pLarge;

    std::cout << "Func 9: assign to null pointer - seg faults" << std::endl;
    cp = new char;
    *cp = 'a'; // err - used null pointer (Seg fauilts)。如果没有前一句分配，这里会直接宕掉
    delete cp;

    std::cout << "End of tests" << std::endl;

    return 0;
}
