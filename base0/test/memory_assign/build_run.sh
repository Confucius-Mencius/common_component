#!/bin/bash

if [ -f ./memory_assign_test ]; then
    rm -f memory_assign_test
fi

gcc -o memory_assign_test memory_assign_test.c

readelf -a memory_assign_test > 1.txt
./memory_assign_test >3.txt
