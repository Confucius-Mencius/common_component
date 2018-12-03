#include "c_api_module1.h"

double g_foo = 3.0;

int SomeFunc(int x, int y)
{
    int g;
    g = y;

    while (x > 0)
    {
        g = x;
        x = y % x;
        y = g;
    }

    return g;
}
