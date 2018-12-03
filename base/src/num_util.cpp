#include "num_util.h"

bool Multiply(i32& ret, i32 a, i32 b)
{
    /*
     * 两个I32_MAX相乘的结果为：4611686014132420609，该值的2倍为：
     * 9223372028264841218，I64_MAX为：
     * 9223372036854775807
     */
    if (a <= 0 || b <= 0)
    {
        return false;
    }

    const i64 i1 = a;
    const i64 i2 = b;
    const i64 limit = I32_MAX;

    const i64 i = i1 * i2;
    if (i > limit)
    {
        return false;
    }

    ret = (i32) i;
    return true;
}
