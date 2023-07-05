#!/bin/bash

cmake -S. -Bcmake-build-debug -DCMAKE_BUILD_TYPE=Debug -DAPPLE_IOS=TRUE
cmake --build cmake-build-debug -j 8