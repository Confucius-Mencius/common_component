#!/bin/bash

valgrind --tool=callgrind -separate-threads=yes ./callgrind_test

# 得到一个callgrind文件：callgrind.out.32682 (32682是进程pid)

# 使用callgrind_annotate来解析这个输出文件：
callgrind_annotate --auto=yes callgrind.out.*

# 或者用图形工具kcachegrind打开callgrind.out.*文件
