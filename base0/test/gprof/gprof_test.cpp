/*
 gprof以及gcov的使用
 gprof 以及 gcov 主要用来对程序的执行情况进行分析，找到其中的"热点(hotspot)"，然后再对其进行优化，以达到全局优化的目的。
 下面是一个比较极端的例子，使用了两者的基本功能。
 #include <stdlib.h>
 #include <stdio.h>

 int prime(int num);

 int main() {
 int i, colcnt = 0;
 for (i = 2; i < 50000; ++i) {
 if (prime(i)) {
 colcnt++;
 if (colcnt % 9 == 0) {
 printf("%5d\n", i);
 colcnt = 0;
 } else
 printf("%5d ", i);
 }
 }
 putchar('\n');
 return 0;
 }

 int prime(int num) {
 int i;
 for (i = 2; i < num; ++i)
 if (num % i == 0)
 return 0;
 return 1;
 }

 编译选项
 # `-pg` for gprof
 # `-fprofile-arcs' and `-ftest-coverage' for gcov
 gcc -pg -fprofile-arcs -ftest-coverage -o sample1.out sample1.c

 先运行一遍程序,然后再运行 gprof
 $ ./sample1.out
 $ gprof -b ./sample1.out

 得到输出为
 Flat profile:

 Each sample counts as 0.01 seconds.
 %   cumulative   self              self     total
 time   seconds   seconds    calls  us/call  us/call  name
 100.13      1.79     1.79    49998    35.85    35.85  prime

 Call graph


 granularity: each sample hit covers 2 byte(s) for 0.56% of 1.79 seconds

 index % time    self  children    called     name
 1.79    0.00   49998/49998       main [2]
 [1]    100.0    1.79    0.00   49998         prime [1]
 -----------------------------------------------
 <spontaneous>
 [2]    100.0    0.00    1.79                 main [2]
 1.79    0.00   49998/49998       prime [1]
 -----------------------------------------------

 Index by function name

 [1] prime

 此处可以看出每个函数的调用次数以及所耗费的时间。
 然后再来看看 gcov 的使用。加上上面的编译选项后，编译完后会生成一个 sample1.gcno 文件，这是个二进制文件，暂且不论。 先运行程序(也可以直接在前面一次运行的基础上继续，无需再次运行)，运行完成后，我们可以看到还会生成一个 sample1.gcda 文件，同样它还是一个二进制文件，这两个文件包含的都是辅助信息，帮助 gcov 来生成相关的代码运行情况。
 $ ./sample1.out
 $ gcov sample1.c

 这样会生成 sample1.c.gcov 文件
 -:    0:Source:sample1.c
 -:    0:Graph:sample1.gcno
 -:    0:Data:sample1.gcda
 -:    0:Runs:1
 -:    0:Programs:1
 -:    1:#include <stdlib.h>
 -:    2:#include <stdio.h>
 -:    3:
 -:    4:int prime(int num);
 -:    5:
 1:    6:int main() {
 1:    7:    int i, colcnt = 0;
 49999:    8:    for (i = 2; i < 50000; ++i) {
 49998:    9:        if (prime(i)) {
 5133:   10:            colcnt++;
 5133:   11:            if (colcnt % 9 == 0) {
 570:   12:                printf("%5d\n", i);
 570:   13:                colcnt = 0;
 -:   14:            } else
 4563:   15:                printf("%5d ", i);
 -:   16:            }
 -:   17:    }
 1:   18:    putchar('\n');
 1:   19:    return 0;
 -:   20:}
 -:   21:
 49998:   22:int prime(int num) {
 -:   23:    int i;
 121337003:   24:    for (i = 2; i < num; ++i)
 121331870:   25:        if (num % i == 0)
 44865:   26:            return 0;
 5133:   27:    return 1;
 -:   28:}

 第一列就是每行代码的执行次数，可以看到第24、25行的执行次数巨大，需要动刀子。
 对 prime 函数做一点修改，实际上并不需要做那么多循环，可以通过 sqrt 函数过滤掉一些
 #include <math.h>
 int faster(int num) {
 return (int) sqrt((float)num);
 }

 编译时加入 -lm 选项
 gcc -pg -fprofile-arcs -ftest-coverage -o sample2.out sample2.c -lm

 同样，按照上面的步骤，gprof 的输出为
 Flat profile:

 Each sample counts as 0.01 seconds.
 %   cumulative   self              self     total
 time   seconds   seconds    calls  ns/call  ns/call  name
 50.06      0.01     0.01  1055977     9.48     9.48  faster
 50.06      0.02     0.01    49998   200.26   400.52  prime

 Call graph


 granularity: each sample hit covers 2 byte(s) for 49.94% of 0.02 seconds

 index % time    self  children    called     name
 0.01    0.01   49998/49998       main [2]
 [1]    100.0    0.01    0.01   49998         prime [1]
 0.01    0.00 1055977/1055977     faster [3]
 -----------------------------------------------
 <spontaneous>
 [2]    100.0    0.00    0.02                 main [2]
 0.01    0.01   49998/49998       prime [1]
 -----------------------------------------------
 0.01    0.00 1055977/1055977     prime [1]
 [3]     50.0    0.01    0.00 1055977         faster [3]
 -----------------------------------------------

 Index by function name

 [3] faster                  [1] prime

 运行时间锐减到0.02秒。再看 gcov的情况
 -:    0:Source:sample2.c
 -:    0:Graph:sample2.gcno
 -:    0:Data:sample2.gcda
 -:    0:Runs:1
 -:    0:Programs:1
 -:    1:#include <stdlib.h>
 -:    2:#include <stdio.h>
 -:    3:#include <math.h>
 -:    4:
 -:    5:int prime(int num);
 -:    6:int faster(int num);
 -:    7:
 1:    8:int main() {
 1:    9:    int i, colcnt = 0;
 49999:   10:    for (i = 2; i < 50000; ++i) {
 49998:   11:        if (prime(i)) {
 5198:   12:            colcnt++;
 5198:   13:            if (colcnt % 9 == 0) {
 577:   14:                printf("%5d\n", i);
 577:   15:                colcnt = 0;
 -:   16:            } else
 4621:   17:                printf("%5d ", i);
 -:   18:        }
 -:   19:    }
 1:   20:    putchar('\n');
 1:   21:    return 0;
 -:   22:}
 -:   23:
 49998:   24:int prime(int num) {
 -:   25:    int i;
 1055977:   26:    for (i = 2; i < faster(num); ++i)
 1050779:   27:        if (num % i == 0)
 44800:   28:            return 0;
 5198:   29:    return 1;
 -:   30:}
 -:   31:
 1055977:   32:int faster(int num) {
 1055977:   33:    return (int) sqrt((float)num);
 -:   34:}

 热点代码的运行次数减降低了2个数量级，这是个不小的收获。
 如果对GCC加 -O3 的编译选项，那么 gprof 可能得不到完整的信息输出，可能是编译器优化打乱了代码的顺序执行而干扰了 gprof的分析。但是 gcov 却依然可以得到结果，只不过会多输出几个头文件的分析结果。下面是加了 -O3 后的结果
 -:    0:Source:sample2.c
 -:    0:Graph:sample2.gcno
 -:    0:Data:sample2.gcda
 -:    0:Runs:1
 -:    0:Programs:1
 -:    1:#include <stdlib.h>
 -:    2:#include <stdio.h>
 -:    3:#include <math.h>
 -:    4:
 -:    5:int prime(int num);
 -:    6:int faster(int num);
 -:    7:
 1:    8:int main() {
 -:    9:    int i, colcnt = 0;
 49999:   10:    for (i = 2; i < 50000; ++i) {
 49998:   11:        if (prime(i)) {
 5198:   12:            colcnt++;
 5198:   13:            if (colcnt % 9 == 0) {
 -:   14:                printf("%5d\n", i);
 -:   15:                colcnt = 0;
 -:   16:            } else
 -:   17:                printf("%5d ", i);
 -:   18:        }
 -:   19:    }
 -:   20:    putchar('\n');
 1:   21:    return 0;
 -:   22:}
 -:   23:
 49998:   24:int prime(int num) {
 -:   25:    int i;
 1055977:   26:    for (i = 2; i < faster(num); ++i)
 1050779:   27:        if (num % i == 0)
 -:   28:            return 0;
 -:   29:    return 1;
 -:   30:}
 -:   31:
 1055977:   32:int faster(int num) {
 1055977:   33:    return (int) sqrt((float)num);
 -:   34:}

 从结果可以看出，一些语句被优化掉了，但关键步骤的代码执行次数依然不变。
 另外，还有两个文件，其中一个文件中显示了 printf 的调用次数。
 # stdio2.h.gcov
 -:  103:printf (__const char *__restrict __fmt, ...)
 -:  104:{
 577:  105:  return __printf_chk (__USE_FORTIFY_LEVEL - 1, __fmt, __va_arg_pack ());
 -:  106:}

 其实 gprof 以及 gcov 还能输出图形结果，那样的结果更具可读性，而且分析起来也比较直观。

 */
#include <stdlib.h>
#include <stdio.h>

int prime(int num)
{
    for (int i = 2; i < num; ++i)
    {
        if (num % i == 0)
        {
            return 0;
        }
    }

    return 1;
}

int main()
{
    int colcnt = 0;

    for (int i = 2; i < 50000; ++i)
    {
        if (prime(i))
        {
            colcnt++;
            if (colcnt % 9 == 0)
            {
                printf("%5d\n", i);
                colcnt = 0;
            }
            else
            {
                printf("%5d ", i);
            }
        }
    }

    putchar('\n');
    return 0;
}
