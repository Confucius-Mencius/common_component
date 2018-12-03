#!/bin/bash

valgrind --tool=massif ./massif_test

# 得到一个massif文件：massif.out.32682 (32682是进程pid)

# 使用ms_print来解析这个输出文件：
ms_print massif.out.*
