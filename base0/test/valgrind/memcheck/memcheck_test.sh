#!/bin/bash

valgrind --tool=memcheck --leak-check=full --error-limit=no --track-origins=yes --trace-children=yes --track-fds=yes --show-reachable=yes --read-var-info=yes -v --log-file=memcheck.log ./memcheck_test
